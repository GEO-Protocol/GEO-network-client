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
                        mTrustLines
                    );
                    break;
                }

                case BaseTransaction::TransactionType::SetTrustLineTransactionType: {
                    transaction = new SetTrustLineTransaction(
                        transactionBuffer,
                        mTrustLines
                    );
                    break;
                }

                case BaseTransaction::TransactionType::CloseTrustLineTransactionType: {
                    transaction = new CloseTrustLineTransaction(
                        transactionBuffer,
                        mTrustLines
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


    if (message->typeID() == Message::AcceptTrustLineMessageType) {
        launchAcceptTrustLineTransaction(
            static_pointer_cast<AcceptTrustLineMessage>(message));

    } else if (message->typeID() == Message::RejectTrustLineMessageType) {
        launchRejectTrustLineTransaction(
            static_pointer_cast<RejectTrustLineMessage>(message));

    } else if (message->typeID() == Message::UpdateTrustLineMessageType) {
        launchUpdateTrustLineTransaction(
            static_pointer_cast<UpdateTrustLineMessage>(message));

    } else if (message->typeID() == Message::MessageTypeID::FirstLevelRoutingTableIncomingMessageType) {
        launchAcceptFromInitiatorToContractorRoutingTablesTransaction(
            static_pointer_cast<FirstLevelRoutingTableIncomingMessage>(message));

    } else if (message->typeID() == Message::MessageTypeID::ReceiveMaxFlowCalculationOnTargetMessageType) {
        launchReceiveMaxFlowCalculationTransaction(
            static_pointer_cast<ReceiveMaxFlowCalculationOnTargetMessage>(message));

    } else if (message->typeID() == Message::MessageTypeID::ResultMaxFlowCalculationMessageType) {
        launchReceiveResultMaxFlowCalculationTransaction(
            static_pointer_cast<ResultMaxFlowCalculationMessage>(message));

    } else if (message->typeID() == Message::MessageTypeID::MaxFlowCalculationSourceFstLevelInMessageType) {
        launchMaxFlowCalculationSourceFstLevelTransaction(
            static_pointer_cast<MaxFlowCalculationSourceFstLevelInMessage>(message));

    } else if (message->typeID() == Message::MessageTypeID::MaxFlowCalculationTargetFstLevelInMessageType) {
        launchMaxFlowCalculationTargetFstLevelTransaction(
            static_pointer_cast<MaxFlowCalculationTargetFstLevelInMessage>(message));

    } else if (message->typeID() == Message::MessageTypeID::MaxFlowCalculationSourceSndLevelInMessageType) {
        launchMaxFlowCalculationSourceSndLevelTransaction(
            static_pointer_cast<MaxFlowCalculationSourceSndLevelInMessage>(message));

    } else if (message->typeID() == Message::MessageTypeID::MaxFlowCalculationTargetSndLevelInMessageType) {
        launchMaxFlowCalculationTargetSndLevelTransaction(
            static_pointer_cast<MaxFlowCalculationTargetSndLevelInMessage>(message));

    } else if (message->typeID() == Message::Payments_ReceiverInitPayment) {
        launchReceiverPaymentTransaction(
            static_pointer_cast<ReceiverInitPaymentMessage>(message));

    } else if (message->typeID() == Message::MessageTypeID::InBetweenNodeTopologyMessage){
        launchGetTopologyAndBalancesTransaction(
            static_pointer_cast<InBetweenNodeTopologyMessage>(message));

    } else if (message->typeID() == Message::MessageTypeID::BoundaryNodeTopologyMessage){
        launchGetTopologyAndBalancesTransaction(
            static_pointer_cast<BoundaryNodeTopologyMessage>(message));

    } else {
        mScheduler->tryAttachMessageToTransaction(message);
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
void TransactionsManager::launchAcceptTrustLineTransaction(
    AcceptTrustLineMessage::Shared message) {

    try {
        auto transaction = make_shared<AcceptTrustLineTransaction>(
            mNodeUUID,
            message,
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
void TransactionsManager::launchFromInitiatorToContractorRoutingTablePropagationTransaction(
    const NodeUUID &contractorUUID,
    const TrustLineDirection direction) {

    try {

        auto transaction = make_shared<FromInitiatorToContractorRoutingTablePropagationTransaction>(
            mNodeUUID,
            contractorUUID,
            mTrustLines
        );

        subscribeForOutgoingMessages(
            transaction->outgoingMessageIsReadySignal);

        mScheduler->postponeTransaction(
            transaction,
            5000);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchFromInitiatorToContractorRoutingTablePropagationTransaction: "
                "can't allocate memory for transaction instance.");
    }
}

void TransactionsManager::launchAcceptFromInitiatorToContractorRoutingTablesTransaction(
    FirstLevelRoutingTableIncomingMessage::Shared message) {

    try {

        auto transaction = make_shared<FromInitiatorToContractorRoutingTablesAcceptTransaction>(
            mNodeUUID,
            message
        );

        subscribeForOutgoingMessages(
            transaction->outgoingMessageIsReadySignal);

        mScheduler->scheduleTransaction(
            transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchAcceptFromInitiatorToContractorRoutingTablesTransaction: "
                "can't allocate memory for transaction instance.");
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
void TransactionsManager::launchReceiveResultMaxFlowCalculationTransaction(
    ResultMaxFlowCalculationMessage::Shared message) {

    try {
        auto transaction = make_shared<ReceiveResultMaxFlowCalculationTransaction>(
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
            "TransactionsManager::launchReceiveResultMaxFlowCalculationTransaction: "
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

void TransactionsManager::launchGetTopologyAndBalancesTransaction(
    InBetweenNodeTopologyMessage::Shared message){
    try {
        auto transaction = make_shared<GetTopologyAndBalancesTransaction>(
            BaseTransaction::TransactionType::GetTopologyAndBalancesTransaction,
            mNodeUUID,
            message,
            mScheduler.get(),
            mTrustLines,
            mLog
        );

//        todo add body

        mScheduler->scheduleTransaction(transaction);
        cout << "launchGetTopologyAndBalancesTransaction" << endl;

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchOpenTrustLineTransaction: "
                "Can't allocate memory for transaction instance.");
    }
}

void TransactionsManager::launchGetTopologyAndBalancesTransaction(){

    try {
        auto transaction = make_shared<GetTopologyAndBalancesTransaction>(
            BaseTransaction::TransactionType::GetTopologyAndBalancesTransaction,
            mNodeUUID,
            mScheduler.get(),
            mTrustLines,
            mLog
        );

//        todo add body

        mScheduler->scheduleTransaction(transaction);
        cout << "launchGetTopologyAndBalancesTransaction" << endl;

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchOpenTrustLineTransaction: "
                "Can't allocate memory for transaction instance.");
    }
}

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