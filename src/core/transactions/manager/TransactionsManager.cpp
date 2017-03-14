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
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    ResultsInterface *resultsInterface,
    history::OperationsHistoryStorage *operationsHistoryStorage,
    Logger *logger) :

    mNodeUUID(nodeUUID),
    mIOService(IOService),
    mTrustLines(trustLinesManager),
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager),
    mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager),
    mResultsInterface(resultsInterface),
    mOperationsHistoryStorage(operationsHistoryStorage),
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
                        mTrustLines,
                        mOperationsHistoryStorage
                    );
                    break;
                }

                case BaseTransaction::TransactionType::SetTrustLineTransactionType: {
                    transaction = new SetTrustLineTransaction(
                        transactionBuffer,
                        mTrustLines,
                        mOperationsHistoryStorage
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
            dynamic_pointer_cast<CreditUsageCommand>(
                command));

    } else if (command->identifier() == InitiateMaxFlowCalculationCommand::identifier()){
        launchInitiateMaxFlowCalculatingTransaction(
            static_pointer_cast<InitiateMaxFlowCalculationCommand>(
                command));

    } else {
        throw ValueError(
            "TransactionsManager::processCommand: "
                "Unexpected command identifier.");
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
        launchAcceptRoutingTablesTransaction(
            static_pointer_cast<FirstLevelRoutingTableIncomingMessage>(message));

    } else if (message->typeID() == Message::MessageTypeID::RoutingTableUpdateIncomingMessageType) {
        launchAcceptRoutingTablesUpdatesTransaction(
            static_pointer_cast<RoutingTableUpdateIncomingMessage>(message));

    } else if (message->typeID() == Message::MessageTypeID::InitiateMaxFlowCalculationMessageType) {
        launchReceiveMaxFlowCalculationOnTargetTransaction(
            static_pointer_cast<InitiateMaxFlowCalculationMessage>(message));

    } else if (message->typeID() == Message::MessageTypeID::ResultMaxFlowCalculationMessageType) {
        launchReceiveResultMaxFlowCalculationTransaction(
            static_pointer_cast<ResultMaxFlowCalculationMessage>(message));

    } else if (message->typeID() == Message::MessageTypeID::MaxFlowCalculationSourceFstLevelMessageType) {
        launchMaxFlowCalculationSourceFstLevelTransaction(
            static_pointer_cast<MaxFlowCalculationSourceFstLevelMessage>(message));

    } else if (message->typeID() == Message::MessageTypeID::MaxFlowCalculationTargetFstLevelMessageType) {
        launchMaxFlowCalculationTargetFstLevelTransaction(
            static_pointer_cast<MaxFlowCalculationTargetFstLevelMessage>(message));

    } else if (message->typeID() == Message::MessageTypeID::MaxFlowCalculationSourceSndLevelMessageType) {
        launchMaxFlowCalculationSourceSndLevelTransaction(
            static_pointer_cast<MaxFlowCalculationSourceSndLevelMessage>(message));

    } else if (message->typeID() == Message::MessageTypeID::MaxFlowCalculationTargetSndLevelMessageType) {
        launchMaxFlowCalculationTargetSndLevelTransaction(
            static_pointer_cast<MaxFlowCalculationTargetSndLevelMessage>(message));

    /*
     * Payments
     */
    } else if (message->typeID() == Message::Payments_ReceiverInitPaymentRequest) {
        launchReceiverPaymentTransaction(
            static_pointer_cast<ReceiverInitPaymentRequestMessage>(message));

    } else if (message->typeID() == Message::Payments_IntermediateNodeReservationRequest) {
        // It is possible, that transaction was already initialised
        // by the ReceiverInitPaymentRequest.
        // In this case - message must be simply attached to it,
        // no new transaction must be launched.
        try {
            mScheduler->tryAttachMessageToTransaction(message);

        } catch (NotFoundError &) {
            launchIntermediateNodePaymentTransaction(
                static_pointer_cast<IntermediateNodeReservationRequestMessage>(message));
        }

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
            mTrustLines,
            mOperationsHistoryStorage
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
            mTrustLines,
            mOperationsHistoryStorage
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
            mTrustLines,
            mOperationsHistoryStorage
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
            mTrustLines,
            mOperationsHistoryStorage
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

        auto transaction = make_shared<FromInitiatorToContractorRoutingTablesPropagationTransaction>(
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

void TransactionsManager::launchAcceptRoutingTablesTransaction(
    FirstLevelRoutingTableIncomingMessage::Shared message) {

    try {

        BaseTransaction::Shared transaction;

        switch(message->propagationStep()) {

            case RoutingTablesMessage::PropagationStep::FromInitiatorToContractor: {

                transaction = dynamic_pointer_cast<BaseTransaction>(
                    make_shared<FromInitiatorToContractorRoutingTablesAcceptTransaction>(
                        mNodeUUID,
                        message,
                        mTrustLines
                    )
                );
                break;
            }

            case RoutingTablesMessage::PropagationStep::FromContractorToFirstLevel: {

                transaction = dynamic_pointer_cast<BaseTransaction>(
                    make_shared<FromContractorToFirstLevelRoutingTablesAcceptTransaction>(
                        mNodeUUID,
                        message,
                        mTrustLines
                    )
                );
                break;
            }

            case RoutingTablesMessage::PropagationStep::FromFirstLevelToSecondLevel: {

                transaction = dynamic_pointer_cast<BaseTransaction>(
                    make_shared<FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction>(
                        mNodeUUID,
                        message
                    )
                );
                break;
            }

            default: {
                throw ValueError(
                    "TransactionsManager::launchAcceptRoutingTablesTransaction: "
                        "Unexpected routing table propagation step.");
            }

        }

        subscribeForSubsidiaryTransactions(
            transaction->runSubsidiaryTransactionSignal);

        subscribeForOutgoingMessages(
            transaction->outgoingMessageIsReadySignal);

        mScheduler->scheduleTransaction(
            transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchAcceptRoutingTablesTransaction: "
                "can't allocate memory for transaction instance.");
    }
}

void TransactionsManager::launchRoutingTablesUpdatingTransactionsFactory(
    const NodeUUID &contractorUUID,
    const TrustLineDirection direction) {

    auto transaction = dynamic_pointer_cast<BaseTransaction>(
        make_shared<RoutingTablesUpdateTransactionsFactory>(
            mNodeUUID,
            contractorUUID,
            direction,
            mTrustLines));

    subscribeForSubsidiaryTransactions(
        transaction->runSubsidiaryTransactionSignal);

    subscribeForOutgoingMessages(
        transaction->outgoingMessageIsReadySignal);

    mScheduler->postponeTransaction(
        transaction,
        5000);
}

void TransactionsManager::launchAcceptRoutingTablesUpdatesTransaction(
    RoutingTableUpdateIncomingMessage::Shared message) {

    auto transaction = dynamic_pointer_cast<BaseTransaction>(
        make_shared<AcceptRoutingTablesUpdatesTransaction>(
            mNodeUUID,
            message,
            mTrustLines));

    subscribeForSubsidiaryTransactions(
        transaction->runSubsidiaryTransactionSignal);

    subscribeForOutgoingMessages(
        transaction->outgoingMessageIsReadySignal);

    mScheduler->scheduleTransaction(
        transaction);
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
            mMaxFlowCalculationCacheManager,
            mLog);

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
void TransactionsManager::launchReceiveMaxFlowCalculationOnTargetTransaction(
        InitiateMaxFlowCalculationMessage::Shared message) {

    try {
        auto transaction = make_shared<ReceiveMaxFlowCalculationOnTargetTransaction>(
            mNodeUUID,
            message,
            mTrustLines,
            mMaxFlowCalculationCacheManager,
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
    MaxFlowCalculationSourceFstLevelMessage::Shared message) {

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
    MaxFlowCalculationTargetFstLevelMessage::Shared message) {

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
    MaxFlowCalculationSourceSndLevelMessage::Shared message) {

    try {
        auto transaction = make_shared<MaxFlowCalculationSourceSndLevelTransaction>(
            mNodeUUID,
            message,
            mTrustLines,
            mMaxFlowCalculationCacheManager,
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
    MaxFlowCalculationTargetSndLevelMessage::Shared message) {

    try {
        auto transaction = make_shared<MaxFlowCalculationTargetSndLevelTransaction>(
            mNodeUUID,
            message,
            mTrustLines,
            mMaxFlowCalculationCacheManager,
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

void TransactionsManager::launchMaxFlowCalculationCacheUpdateTransaction() {

    try {
        auto transaction = make_shared<MaxFlowCalculationCacheUpdateTransaction>(
            mNodeUUID,
            mMaxFlowCalculationCacheManager,
            mMaxFlowCalculationTrustLineManager,
            mLog);

        subscribeForOutgoingMessages(
            transaction->outgoingMessageIsReadySignal);

        mScheduler->scheduleTransaction(
            transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchMaxFlowCalculationCacheUpdateTransaction: "
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

void TransactionsManager::launchReceiverPaymentTransaction(
    ReceiverInitPaymentRequestMessage::Shared message)
{
    prepeareAndSchedule(
        make_shared<ReceiverPaymentTransaction>(
            mNodeUUID,
            message,
            mTrustLines,
            mLog));
}

void TransactionsManager::launchIntermediateNodePaymentTransaction(
    IntermediateNodeReservationRequestMessage::Shared message)
{
    prepeareAndSchedule(
        make_shared<IntermediateNodePaymentTransaction>(
            mNodeUUID,
            message,
            mTrustLines,
            mLog));
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

void TransactionsManager::subscribeForSubsidiaryTransactions(
    BaseTransaction::LaunchSubsidiaryTransactionSignal &signal) {

    signal.connect(
        boost::bind(
            &TransactionsManager::onSubsidiaryTransactionReady,
            this,
            _1
        )
    );
}

void TransactionsManager::subscribeForOutgoingMessages(
    BaseTransaction::SendMessageSignal &signal) {

    signal.connect(
        boost::bind(
            &TransactionsManager::onTransactionOutgoingMessageReady,
            this,
            _1,
            _2
        )
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

void TransactionsManager::onSubsidiaryTransactionReady(
    BaseTransaction::Shared transaction) {

    subscribeForSubsidiaryTransactions(
        transaction->runSubsidiaryTransactionSignal
    );

    subscribeForOutgoingMessages(
        transaction->outgoingMessageIsReadySignal
    );

    mScheduler->postponeTransaction(
        transaction,
        5000
    );
}

/**
 *
 * @throws bad_alloc;
 */
void TransactionsManager::prepeareAndSchedule(
    BaseTransaction::Shared transaction)
{
    subscribeForOutgoingMessages(
        transaction->outgoingMessageIsReadySignal);

    mScheduler->scheduleTransaction(
        transaction);
}
