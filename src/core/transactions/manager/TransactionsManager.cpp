#include "TransactionsManager.h"

/*!
 *
 * Throws RuntimeError in case if some internal components can't be initialised.
 */
TransactionsManager::TransactionsManager(
    NodeUUID &nodeUUID,
    as::io_service &IOService,
    TrustLinesManager *trustLinesManager,
    MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
    ResultsInterface *resultsInterface,
    Logger *logger) :

    mNodeUUID(nodeUUID),
    mIOService(IOService),
    mTrustLines(trustLinesManager),
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager),
    mResultsInterface(resultsInterface),
    mLog(logger),

    mStorage(new storage::UUIDMapBlockStorage(
        "io/transactions",
        "transactions.dat")
    ),

    mScheduler(new TransactionsScheduler(
        mIOService,
        mStorage.get(),
        mLog)
    ) {

    subscribeForCommandResult(mScheduler->commandResultIsReadySignal);

    try {
        loadTransactions();

    } catch (exception &e) {
        throw RuntimeError(e.what());
    }
}

/*!
 *
 * Throws ValueError in case if command is unexpected.
 */
void TransactionsManager::processCommand(
    BaseUserCommand::Shared command) {

    if (command->identifier() == OpenTrustLineCommand::identifier()) {
        launchOpenTrustLineTransaction(
            static_pointer_cast<OpenTrustLineCommand>(
                command));

    } else if (command->identifier() == CloseTrustLineCommand::identifier()) {
        launchCloseTrustLineTransaction(
            static_pointer_cast<CloseTrustLineCommand>(
                command));

    } else if (command->identifier() == SetTrustLineCommand::identifier()) {
        launchSetTrustLineTransaction(
            static_pointer_cast<SetTrustLineCommand>(
                command));

    } else if (command->identifier() == CreditUsageCommand::identifier()) {
        launchCoordinatorPaymentTransaction(
            static_pointer_cast<CreditUsageCommand>(
                command));

    } else if (command->identifier() == InitiateMaxFlowCalculationCommand::identifier()){
        launchInitiateMaxFlowCalculatingTransaction(
            static_pointer_cast<InitiateMaxFlowCalculationCommand>(
                command));

    } else {
        throw ValueError(
            "TransactionsManager::processCommand: "
                "unexpected command identifier.");
    }
}

void TransactionsManager::processMessage(
    Message::Shared message) {

    if (message->typeID() == Message::MessageTypeID::AcceptTrustLineMessageType) {
        launchAcceptTrustLineTransaction(
            static_pointer_cast<AcceptTrustLineMessage>(
                message
            )
        );

    } else if (message->typeID() == Message::MessageTypeID::RejectTrustLineMessageType) {
        launchRejectTrustLineTransaction(
            static_pointer_cast<RejectTrustLineMessage>(
                message
            )
        );

    } else if (message->typeID() == Message::MessageTypeID::UpdateTrustLineMessageType) {
        launchUpdateTrustLineTransaction(
            static_pointer_cast<UpdateTrustLineMessage>(
                message
            )
        );

    } else if (message->typeID() == Message::MessageTypeID::ReceiveMaxFlowCalculationOnTargetMessageType) {
        launchReceiveMaxFlowCalculationTransaction(
            static_pointer_cast<ReceiveMaxFlowCalculationOnTargetMessage>(
                message
            )
        );

    } else if (message->typeID() == Message::MessageTypeID::ResultMaxFlowCalculationFromTargetMessageType) {
        launchReceiveResultMaxFlowCalculationFromTargetTransaction(
            static_pointer_cast<ResultMaxFlowCalculationFromTargetMessage>(
                message
            )
        );

    } else if (message->typeID() == Message::MessageTypeID::ResultMaxFlowCalculationFromSourceMessageType) {
        launchReceiveResultMaxFlowCalculationFromSourceTransaction(
            static_pointer_cast<ResultMaxFlowCalculationFromSourceMessage>(
                message
            )
        );

    } else if (message->typeID() == Message::MessageTypeID::MaxFlowCalculationSourceFstLevelInMessageType) {
        launchMaxFlowCalculationSourceFstLevelTransaction(
            static_pointer_cast<MaxFlowCalculationSourceFstLevelInMessage>(
                message
            )
        );

    } else if (message->typeID() == Message::MessageTypeID::MaxFlowCalculationTargetFstLevelInMessageType) {
        launchMaxFlowCalculationTargetFstLevelTransaction(
            static_pointer_cast<MaxFlowCalculationTargetFstLevelInMessage>(
                message
            )
        );

    } else if (message->typeID() == Message::MessageTypeID::MaxFlowCalculationSourceSndLevelInMessageType) {
        launchMaxFlowCalculationSourceSndLevelTransaction(
            static_pointer_cast<MaxFlowCalculationSourceSndLevelInMessage>(
                message
            )
        );

    } else if (message->typeID() == Message::MessageTypeID::MaxFlowCalculationTargetSndLevelInMessageType) {
        launchMaxFlowCalculationTargetSndLevelTransaction(
            static_pointer_cast<MaxFlowCalculationTargetSndLevelInMessage>(
                message
            )
        );

    } else {
        mScheduler->handleMessage(message);
    }
}

void TransactionsManager::loadTransactions() {

    auto uuidKeys = unique_ptr<const vector<storage::uuids::uuid>>(mStorage->keys());
    if (uuidKeys->size() == 0) {
        return;
    }

    for (auto const &uuidKey : *uuidKeys) {
        try {
            auto record = mStorage->readByUUID(uuidKey);

            BytesShared transactionBuffer = tryCalloc(record->bytesCount());
            memcpy(
                transactionBuffer.get(),
                const_cast<byte *> (record->data()),
                record->bytesCount()
            );

            BaseTransaction::SerializedTransactionType *type = new (transactionBuffer.get()) BaseTransaction::SerializedTransactionType;
            BaseTransaction::TransactionType transactionType = (BaseTransaction::TransactionType) *type;

            BaseTransaction *transaction;
            switch (transactionType) {
                case BaseTransaction::TransactionType::OpenTrustLineTransactionType: {
                    transaction = new OpenTrustLineTransaction(
                        transactionBuffer,
                        mScheduler.get(),
                        mTrustLines
                    );
                    break;
                }

                case BaseTransaction::TransactionType::SetTrustLineTransactionType: {
                    transaction = new SetTrustLineTransaction(
                        transactionBuffer,
                        mScheduler.get(),
                        mTrustLines
                    );
                    break;
                }

                case BaseTransaction::TransactionType::CloseTrustLineTransactionType: {
                    transaction = new CloseTrustLineTransaction(
                        transactionBuffer,
                        mScheduler.get(),
                        mTrustLines
                    );
                    break;
                }

                case BaseTransaction::TransactionType ::InitiateMaxFlowCalculationTransactionType: {
                    transaction = new InitiateMaxFlowCalculationTransaction(
                        transactionBuffer,
                        mTrustLines,
                        mMaxFlowCalculationTrustLineManager
                    );
                    break;
                }

                default: {
                    throw RuntimeError(
                        "TrustLinesManager::loadTransactions. "
                            "Unexpected transaction type identifier.");
                }
            }

            subscribeForOutgoingMessages(
                transaction->outgoingMessageIsReadySignal);

            mScheduler->scheduleTransaction(
                BaseTransaction::Shared(transaction));

        } catch(IndexError &e) {
            throw RuntimeError(
                string(
                    "TransactionsManager::loadTransactions: "
                        "Internal error: ") + e.what());

        } catch (IOError &e) {
            throw RuntimeError(
                string(
                    "TransactionsManager::loadTransactions: "
                        "Internal error: ") + e.what());

        } catch (bad_alloc &) {
            throw MemoryError(
                "TransactionsManager::loadTransactions: "
                    "Bad alloc.");
        }
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchOpenTrustLineTransaction(
    OpenTrustLineCommand::Shared command) {

    try {
        auto transaction = make_shared<OpenTrustLineTransaction>(
            mNodeUUID,
            command,
            mScheduler.get(),
            mTrustLines
        );

        subscribeForOutgoingMessages(transaction->outgoingMessageIsReadySignal);

        mScheduler->scheduleTransaction(transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchOpenTrustLineTransaction: "
                "Can't allocate memory for transaction instance.");
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchSetTrustLineTransaction(
    SetTrustLineCommand::Shared command) {

    try {
        auto transaction = make_shared<SetTrustLineTransaction>(
            mNodeUUID,
            command,
            mScheduler.get(),
            mTrustLines
        );

        subscribeForOutgoingMessages(transaction->outgoingMessageIsReadySignal);

        mScheduler->scheduleTransaction(transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchSetTrustLineTransaction: "
                "Can't allocate memory for transaction instance.");
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchCloseTrustLineTransaction(
    CloseTrustLineCommand::Shared command) {

    try {
        auto transaction = make_shared<CloseTrustLineTransaction>(
            mNodeUUID,
            command,
            mScheduler.get(),
            mTrustLines
        );

        subscribeForOutgoingMessages(transaction->outgoingMessageIsReadySignal);

        mScheduler->scheduleTransaction(transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchCloseTrustLineTransaction: "
                "Can't allocate memory for transaction instance.");
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchInitiateMaxFlowCalculatingTransaction(
    InitiateMaxFlowCalculationCommand::Shared command) {

    try {
        auto transaction = make_shared<InitiateMaxFlowCalculationTransaction>(
            mNodeUUID,
            command,
            mTrustLines,
            mMaxFlowCalculationTrustLineManager,
            mLog
        );

        subscribeForOutgoingMessages(transaction->outgoingMessageIsReadySignal);

        mScheduler->scheduleTransaction(transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchCloseTrustLineTransaction: "
                "Can't allocate memory for transaction instance.");
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchAcceptTrustLineTransaction(
    AcceptTrustLineMessage::Shared message) {

    try {
        auto transaction = make_shared<AcceptTrustLineTransaction>(
            mNodeUUID,
            message,
            mScheduler.get(),
            mTrustLines
        );

        subscribeForOutgoingMessages(transaction->outgoingMessageIsReadySignal);

        mScheduler->scheduleTransaction(transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchAcceptTrustLineTransaction: "
                "Can't allocate memory for transaction instance.");
    }
}

void TransactionsManager::launchUpdateTrustLineTransaction(
    UpdateTrustLineMessage::Shared message) {

    try {
        auto transaction = make_shared<UpdateTrustLineTransaction>(
            mNodeUUID,
            message,
            mScheduler.get(),
            mTrustLines
        );

        subscribeForOutgoingMessages(transaction->outgoingMessageIsReadySignal);

        mScheduler->scheduleTransaction(transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchUpdateTrustLineTransaction: "
                "can't allocate memory for transaction instance.");
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchRejectTrustLineTransaction(
    RejectTrustLineMessage::Shared message) {

    try {
        auto transaction = make_shared<RejectTrustLineTransaction>(
            mNodeUUID,
            message,
            mScheduler.get(),
            mTrustLines
        );

        subscribeForOutgoingMessages(transaction->outgoingMessageIsReadySignal);

        mScheduler->scheduleTransaction(transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchRejectTrustLineTransaction: "
                "Can't allocate memory for transaction instance.");
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchCoordinatorPaymentTransaction(
    CreditUsageCommand::Shared command) {

    try {
        auto transaction = make_shared<CoordinatorPaymentTransaction>(
            mNodeUUID,
            command,
            mTrustLines,
            mLog);

        subscribeForOutgoingMessages(
            transaction->outgoingMessageIsReadySignal);

        mScheduler->scheduleTransaction(
            transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchCoordinatorPaymentTransaction: "
                "can't allocate memory for transaction instance.");
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchReceiverPaymentTransaction(
    ReceiverInitPaymentMessage::Shared message) {

    try {
        auto transaction = make_shared<ReceiverPaymentTransaction>(
            message,
            mTrustLines,
            mLog);

        subscribeForOutgoingMessages(
            transaction->outgoingMessageIsReadySignal);

        mScheduler->scheduleTransaction(
            transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchReceiverPaymentTransaction: "
                "can't allocate memory for transaction instance.");
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchReceiveMaxFlowCalculationTransaction(
    ReceiveMaxFlowCalculationOnTargetMessage::Shared message) {

    try {
        auto transaction = make_shared<ReceiveMaxFlowCalculationOnTargetTransaction>(
            mNodeUUID,
            message,
            mTrustLines,
            mLog);

        subscribeForOutgoingMessages(
            transaction->outgoingMessageIsReadySignal);

        mScheduler->scheduleTransaction(
            transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchReceiverPaymentTransaction: "
                "can't allocate memory for transaction instance.");
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchReceiveResultMaxFlowCalculationFromTargetTransaction(
    ResultMaxFlowCalculationFromTargetMessage::Shared message) {

    try {
        auto transaction = make_shared<ReceiveResultMaxFlowCalculationFromTargetTransaction>(
            mNodeUUID,
            message,
            mTrustLines,
            mMaxFlowCalculationTrustLineManager,
            mLog);

        subscribeForOutgoingMessages(
            transaction->outgoingMessageIsReadySignal);

        mScheduler->scheduleTransaction(
            transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchReceiveResultMaxFlowCalculationFromTargetTransaction: "
                "can't allocate memory for transaction instance.");
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchReceiveResultMaxFlowCalculationFromSourceTransaction(
    ResultMaxFlowCalculationFromSourceMessage::Shared message) {

    try {
        auto transaction = make_shared<ReceiveResultMaxFlowCalculationFromSourceTransaction>(
            mNodeUUID,
            message,
            mTrustLines,
            mMaxFlowCalculationTrustLineManager,
            mLog);

        subscribeForOutgoingMessages(
            transaction->outgoingMessageIsReadySignal);

        mScheduler->scheduleTransaction(
            transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchReceiveResultMaxFlowCalculationFromSourceTransaction: "
                "can't allocate memory for transaction instance.");
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchMaxFlowCalculationSourceFstLevelTransaction(
    MaxFlowCalculationSourceFstLevelInMessage::Shared message) {

    try {
        auto transaction = make_shared<MaxFlowCalculationSourceFstLevelTransaction>(
            mNodeUUID,
            message,
            mTrustLines,
            mLog);

        subscribeForOutgoingMessages(
            transaction->outgoingMessageIsReadySignal);

        mScheduler->scheduleTransaction(
            transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchMaxFlowCalculationSourceFstLevelTransaction: "
                "can't allocate memory for transaction instance.");
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchMaxFlowCalculationTargetFstLevelTransaction(
    MaxFlowCalculationTargetFstLevelInMessage::Shared message) {

    try {
        auto transaction = make_shared<MaxFlowCalculationTargetFstLevelTransaction>(
            mNodeUUID,
            message,
            mTrustLines,
            mLog);

        subscribeForOutgoingMessages(
            transaction->outgoingMessageIsReadySignal);

        mScheduler->scheduleTransaction(
            transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchMaxFlowCalculationTargetFstLevelTransaction: "
                "can't allocate memory for transaction instance.");
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchMaxFlowCalculationSourceSndLevelTransaction(
    MaxFlowCalculationSourceSndLevelInMessage::Shared message) {

    try {
        auto transaction = make_shared<MaxFlowCalculationSourceSndLevelTransaction>(
            mNodeUUID,
            message,
            mTrustLines,
            mLog);

        subscribeForOutgoingMessages(
            transaction->outgoingMessageIsReadySignal);

        mScheduler->scheduleTransaction(
            transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchMaxFlowCalculationSourceSndLevelTransaction: "
                "can't allocate memory for transaction instance.");
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchMaxFlowCalculationTargetSndLevelTransaction(
    MaxFlowCalculationTargetSndLevelInMessage::Shared message) {

    try {
        auto transaction = make_shared<MaxFlowCalculationTargetSndLevelTransaction>(
            mNodeUUID,
            message,
            mTrustLines,
            mLog);

        subscribeForOutgoingMessages(
            transaction->outgoingMessageIsReadySignal);

        mScheduler->scheduleTransaction(
            transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchMaxFlowCalculationSourceSndLevelTransaction: "
                "can't allocate memory for transaction instance.");
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchRoutingTableExchangeTransaction(
    const NodeUUID &contractorUUID,
    const TrustLineDirection direction) {}

void TransactionsManager::subscribeForOutgoingMessages(
    BaseTransaction::SendMessageSignal &signal) {

    signal.connect(
        boost::bind(
            &TransactionsManager::onTransactionOutgoingMessageReady,
            this,
            _1,
            _2)
    );
}

void TransactionsManager::subscribeForCommandResult(
    TransactionsScheduler::CommandResultSignal &signal) {

    signal.connect(
        boost::bind(
            &TransactionsManager::onCommandResultReady,
            this,
            _1
        )
    );
}

void TransactionsManager::onTransactionOutgoingMessageReady(
    Message::Shared message,
    const NodeUUID &contractorUUID) {

    transactionOutgoingMessageReadySignal(
        message,
        contractorUUID);
}

/*!
 * Writes received result to the outgoing results fifo.
 *
 * Throws RuntimeError - in case if result can't be processed.
 */
void TransactionsManager::onCommandResultReady(
    CommandResult::SharedConst result) {

    try {
        auto message = result->serialize();

        mLog->logSuccess(
            "Transactions manager::onCommandResultReady",
            message
        );

        mResultsInterface->writeResult(
            message.c_str(),
            message.size()
        );

    } catch (...) {
        throw RuntimeError(
            "TransactionsManager::onCommandResultReady: "
                "Error occurred when command result has accepted");
    }
}
