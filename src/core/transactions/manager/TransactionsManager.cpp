#include "TransactionsManager.h"

/*!
 *
 * Throws RuntimeError in case if some internal components can't be initialised.
 */
TransactionsManager::TransactionsManager(
    NodeUUID &nodeUUID,
    as::io_service &IOService,
    TrustLinesManager *trustLinesManager,
    ResourcesManager *resourcesManager,
    MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    ResultsInterface *resultsInterface,
    StorageHandler *storageHandler,
    PathsManager *pathsManager,
    Logger *logger) :

    mNodeUUID(nodeUUID),
    mIOService(IOService),
    mTrustLines(trustLinesManager),
    mResourcesManager(resourcesManager),
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager),
    mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager),
    mResultsInterface(resultsInterface),
    mStorageHandler(storageHandler),
    mPathsManager(pathsManager),
    mLog(logger),

    mScheduler(new TransactionsScheduler(
        mIOService,
        mLog))
{

    subscribeForCommandResult(mScheduler->commandResultIsReadySignal);
    subscribeForSerializeTransaction(mScheduler->serializeTransactionSignal);

    try {
        loadTransactions();

    } catch (exception &e) {
        throw RuntimeError(e.what());
    }
}

void TransactionsManager::loadTransactions() {

    const auto ioTransaction = mStorageHandler->beginTransaction();
    const auto serializedTAs = ioTransaction->transactionHandler()->allTransactions();
    for(const auto kTABufferAndSize: serializedTAs) {
        BaseTransaction::SerializedTransactionType *transactionType = new (kTABufferAndSize.first.get()) BaseTransaction::SerializedTransactionType;
        auto TransactionTypeId = *transactionType;
        switch (TransactionTypeId) {
            case BaseTransaction::TransactionType::CoordinatorPaymentTransaction: {
                auto transaction = make_shared<CoordinatorPaymentTransaction>(
                    kTABufferAndSize.first,
                    mNodeUUID,
                    mTrustLines,
                    mStorageHandler,
                    mMaxFlowCalculationCacheManager,
                    mResourcesManager,
                    mLog);
                mScheduler->addTransactionAndState(transaction, TransactionState::awakeAsFastAsPossible());
                break;
            }
            case BaseTransaction::TransactionType::IntermediateNodePaymentTransaction: {
                auto transaction = make_shared<IntermediateNodePaymentTransaction>(
                    kTABufferAndSize.first,
                    mNodeUUID,
                    mTrustLines,
                    mStorageHandler,
                    mMaxFlowCalculationCacheManager,
                    mLog);
                mScheduler->addTransactionAndState(transaction, TransactionState::awakeAsFastAsPossible());
                break;
            }
            case BaseTransaction::TransactionType::ReceiverPaymentTransaction: {
                auto transaction = make_shared<IntermediateNodePaymentTransaction>(
                    kTABufferAndSize.first,
                    mNodeUUID,
                    mTrustLines,
                    mStorageHandler,
                    mMaxFlowCalculationCacheManager,
                    mLog);
                mScheduler->addTransactionAndState(transaction, TransactionState::awakeAsFastAsPossible());
                break;
            }
            default: {
                throw RuntimeError(
                    "TrustLinesManager::loadTransactions. "
                        "Unexpected transaction type identifier.");
            }
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

    } else if (command->identifier() == TotalBalancesCommand::identifier()){
        launchTotalBalancesTransaction(
            static_pointer_cast<TotalBalancesCommand>(
                command));

    } else if (command->identifier() == TotalBalancesRemouteNodeCommand::identifier()){
        launchTotalBalancesRemoteNodeTransaction(
            static_pointer_cast<TotalBalancesRemouteNodeCommand>(
                command));

    } else if (command->identifier() == HistoryPaymentsCommand::identifier()){
        launchHistoryPaymentsTransaction(
            static_pointer_cast<HistoryPaymentsCommand>(
                command));

    } else if (command->identifier() == HistoryTrustLinesCommand::identifier()){
        launchHistoryTrustLinesTransaction(
            static_pointer_cast<HistoryTrustLinesCommand>(
                command));

    } else if (command->identifier() == CycleCloserCommand::identifier()){
        launchTestCloseCycleTransaction(
            static_pointer_cast<CycleCloserCommand>(
                command));

    } else if (command->identifier() == FindPathCommand::identifier()){
        launchGetPathTestTransaction(
            static_pointer_cast<FindPathCommand>(
                command));

    } else if (command->identifier() == GetFirstLevelContractorsCommand::identifier()){
        launchGetFirstLevelContractorsTransaction(
                static_pointer_cast<GetFirstLevelContractorsCommand>(
                        command));

    } else if (command->identifier() == GetTrustLinesCommand::identifier()){
        launchGetTrustlinesTransaction(
            static_pointer_cast<GetTrustLinesCommand>(
                command));

    } else {
        throw ValueError(
            "TransactionsManager::processCommand: "
                "Unexpected command identifier.");
    }
}

void TransactionsManager::processMessage(
    Message::Shared message)
{
    if (message->typeID() == Message::TrustLines_Accept) {
        launchAcceptTrustLineTransaction(
            static_pointer_cast<AcceptTrustLineMessage>(message));

    } else if (message->typeID() == Message::TrustLines_Reject) {
        launchRejectTrustLineTransaction(
            static_pointer_cast<RejectTrustLineMessage>(message));

    } else if (message->typeID() == Message::TrustLines_Update) {
        launchUpdateTrustLineTransaction(
            static_pointer_cast<UpdateTrustLineMessage>(message));

    } else if (message->typeID() == Message::MessageType::MaxFlow_InitiateCalculation) {
        launchReceiveMaxFlowCalculationOnTargetTransaction(
            static_pointer_cast<InitiateMaxFlowCalculationMessage>(message));

    } else if (message->typeID() == Message::MessageType::MaxFlow_ResultMaxFlowCalculation) {
        launchReceiveResultMaxFlowCalculationTransaction(
            static_pointer_cast<ResultMaxFlowCalculationMessage>(message));

    } else if (message->typeID() == Message::MessageType::MaxFlow_CalculationSourceFirstLevel) {
        launchMaxFlowCalculationSourceFstLevelTransaction(
            static_pointer_cast<MaxFlowCalculationSourceFstLevelMessage>(message));

    } else if (message->typeID() == Message::MessageType::MaxFlow_CalculationTargetFirstLevel) {
        launchMaxFlowCalculationTargetFstLevelTransaction(
            static_pointer_cast<MaxFlowCalculationTargetFstLevelMessage>(message));

    } else if (message->typeID() == Message::MessageType::MaxFlow_CalculationSourceSecondLevel) {
        launchMaxFlowCalculationSourceSndLevelTransaction(
            static_pointer_cast<MaxFlowCalculationSourceSndLevelMessage>(message));

    } else if (message->typeID() == Message::MessageType::MaxFlow_CalculationTargetSecondLevel) {
        launchMaxFlowCalculationTargetSndLevelTransaction(
                static_pointer_cast<MaxFlowCalculationTargetSndLevelMessage>(message));

    /*
    * Total balances
    */
    } else if (message->typeID() == Message::MessageType::TotalBalance_Request) {
        launchTotalBalancesTransaction(
                static_pointer_cast<InitiateTotalBalancesMessage>(message));

    } else if (message->typeID() == Message::MessageType::TotalBalance_Response) {
        mScheduler->tryAttachMessageToTransaction(message);

    /*
    * Paths
    */
    } else if (message->typeID() == Message::MessageType::Paths_RequestRoutingTables) {
        launchGetRoutingTablesTransaction(
            static_pointer_cast<RequestRoutingTablesMessage>(message));

    } else if (message->typeID() == Message::MessageType::Paths_ResultRoutingTableFirstLevel) {
        mScheduler->tryAttachMessageToTransaction(message);

    } else if (message->typeID() == Message::MessageType::Paths_ResultRoutingTableSecondLevel) {
        mScheduler->tryAttachMessageToTransaction(message);

    } else if (message->typeID() == Message::MessageType::Paths_ResultRoutingTableThirdLevel) {
        mScheduler->tryAttachMessageToTransaction(message);

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
    } else if(message->typeID() == Message::MessageType::Payments_VotesStatusRequest){
        launchVoutesResponsePaymentsTransaction(
                static_pointer_cast<VotesStatusRequestMessage>(message));
    } else if (message->typeID() == Message::MessageType::Cycles_SixNodesMiddleware) {
        launchSixNodesCyclesResponseTransaction(
                static_pointer_cast<CyclesSixNodesInBetweenMessage>(message));
    } else if (message->typeID() == Message::MessageType::Cycles_FiveNodesMiddleware) {
        launchFiveNodesCyclesResponseTransaction(
            static_pointer_cast<CyclesFiveNodesInBetweenMessage>(message));
    }else if(message->typeID() == Message::MessageType::Cycles_ThreeNodesBalancesRequest){
        launchThreeNodesCyclesResponseTransaction(
                static_pointer_cast<CyclesThreeNodesBalancesRequestMessage>(message));
    }else if(message->typeID() == Message::MessageType::Cycles_FourNodesBalancesRequest){
        launchFourNodesCyclesResponseTransaction(
                static_pointer_cast<CyclesFourNodesBalancesRequestMessage>(message));

    /*
    * Routing tables exchange
    */
    } else if (message->typeID() == Message::MessageType::RoutingTables_NotificationTrustLineCreated) {
        launchTrustLineStatesHandlerTransaction(
            static_pointer_cast<NotificationTrustLineCreatedMessage>(message));

    } else if (message->typeID() == Message::MessageType::RoutingTables_NotificationTrustLineRemoved) {
        launchTrustLineStatesHandlerTransaction(
            static_pointer_cast<NotificationTrustLineRemovedMessage>(message));

    } else if (message->typeID() == Message::MessageType::RoutingTables_NeighborsRequest) {
        launchGetFirstRoutingTableTransaction(
            static_pointer_cast<NeighborsRequestMessage>(message));

    } else if (message->typeID() == Message::MessageType::RoutingTables_NeighborsResponse) {
        mScheduler->tryAttachMessageToTransaction(message);

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
            mStorageHandler,
            mLog);

        subscribeForOutgoingMessages(transaction->outgoingMessageIsReadySignal);
        subscribeForSubsidiaryTransactions(transaction->runSubsidiaryTransactionSignal);

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
            mStorageHandler,
            mLog);

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
            mTrustLines,
            mStorageHandler,
            mLog);

        subscribeForOutgoingMessages(transaction->outgoingMessageIsReadySignal);
        subscribeForSubsidiaryTransactions(transaction->runSubsidiaryTransactionSignal);
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
            mStorageHandler,
            mLog);

        subscribeForOutgoingMessages(transaction->outgoingMessageIsReadySignal);
        subscribeForSubsidiaryTransactions(transaction->runSubsidiaryTransactionSignal);

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
            mStorageHandler,
            mLog);

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
            mTrustLines,
            mStorageHandler,
            mLog);

        subscribeForOutgoingMessages(transaction->outgoingMessageIsReadySignal);
        subscribeForSubsidiaryTransactions(transaction->runSubsidiaryTransactionSignal);
        mScheduler->scheduleTransaction(transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchRejectTrustLineTransaction: "
                "Can't allocate memory for transaction instance.");
    }
}

void TransactionsManager::launchProcessTrustLineModificationTransactions(
    const NodeUUID &contractorUUID)
{
//    prepareAndSchedule(
//        make_shared<>(
//            mNodeUUID,
//            mTrustLines,
//            mLog));
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

        prepareAndSchedule(transaction);

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

        prepareAndSchedule(transaction);

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

        prepareAndSchedule(transaction);

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

        prepareAndSchedule(transaction);

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

        prepareAndSchedule(transaction);

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

        prepareAndSchedule(transaction);

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

        prepareAndSchedule(transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchMaxFlowCalculationSourceSndLevelTransaction: "
                "can't allocate memory for transaction instance.");
    }
}

void TransactionsManager::launchCoordinatorPaymentTransaction(
    CreditUsageCommand::Shared command) {

    prepareAndSchedule(
        make_shared<CoordinatorPaymentTransaction>(
            mNodeUUID,
            command,
            mTrustLines,
            mStorageHandler,
            mMaxFlowCalculationCacheManager,
            mResourcesManager,
            mLog));
}

void TransactionsManager::launchReceiverPaymentTransaction(
    ReceiverInitPaymentRequestMessage::Shared message)
{
    prepareAndSchedule(
        make_shared<ReceiverPaymentTransaction>(
            mNodeUUID,
            message,
            mTrustLines,
            mStorageHandler,
            mMaxFlowCalculationCacheManager,
            mLog));
}

void TransactionsManager::launchIntermediateNodePaymentTransaction(
    IntermediateNodeReservationRequestMessage::Shared message)
{
    prepareAndSchedule(
        make_shared<IntermediateNodePaymentTransaction>(
            mNodeUUID,
            message,
            mTrustLines,
            mStorageHandler,
            mMaxFlowCalculationCacheManager,
            mLog));
}

void TransactionsManager::launchVoutesResponsePaymentsTransaction(
    VotesStatusRequestMessage::Shared message)
{
    prepareAndSchedule(
        make_shared<VotesStatusResponsePaymentTransaction>(
            mNodeUUID,
            message,
            mStorageHandler,
            mLog));
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchTotalBalancesTransaction(
        TotalBalancesCommand::Shared command) {

    try {
        auto transaction = make_shared<TotalBalancesTransaction>(
            mNodeUUID,
            command,
            mTrustLines,
            mLog);

        prepareAndSchedule(transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchTotalBalancesTransaction: "
                "Can't allocate memory for transaction instance.");
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchTotalBalancesTransaction(
        InitiateTotalBalancesMessage::Shared message) {

    try {
        auto transaction = make_shared<TotalBalancesTransaction>(
            mNodeUUID,
            message,
            mTrustLines,
            mLog);

        prepareAndSchedule(transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchTotalBalancesTransaction: "
                "Can't allocate memory for transaction instance.");
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchTotalBalancesRemoteNodeTransaction(
    TotalBalancesRemouteNodeCommand::Shared command) {

    try {
        auto transaction = make_shared<TotalBalancesFromRemoutNodeTransaction>(
            mNodeUUID,
            command,
            mLog);

        prepareAndSchedule(transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchTotalBalancesRemoteNodeTransaction: "
                "Can't allocate memory for transaction instance.");
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchHistoryPaymentsTransaction(
    HistoryPaymentsCommand::Shared command) {
    try {
        auto transaction = make_shared<HistoryPaymentsTransaction>(
            mNodeUUID,
            command,
            mStorageHandler,
            mLog);

        prepareAndSchedule(transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchHistoryPaymentsTransaction: "
                "Can't allocate memory for transaction instance.");
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchHistoryTrustLinesTransaction(
    HistoryTrustLinesCommand::Shared command) {
    try {
        auto transaction = make_shared<HistoryTrustLinesTransaction>(
            mNodeUUID,
            command,
            mStorageHandler,
            mLog);

        prepareAndSchedule(transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchHistoryTrustLinesTransaction: "
                "Can't allocate memory for transaction instance.");
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchGetPathTestTransaction(
    FindPathCommand::Shared command) {
    try {
        auto transaction = make_shared<GetPathTestTransaction>(
            mNodeUUID,
            command,
            mResourcesManager,
            mLog);

        prepareAndSchedule(transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchGetPathTestTransaction: "
                "Can't allocate memory for transaction instance.");
    }
}

void TransactionsManager::launchGetFirstLevelContractorsTransaction(GetFirstLevelContractorsCommand::Shared command)
{
    prepareAndSchedule(
        make_shared<GetFirstLevelContractorsTransaction>(
            mNodeUUID,
            command,
            mTrustLines,
            mLog));
}

void TransactionsManager::launchGetTrustlinesTransaction(GetTrustLinesCommand::Shared command)
{
    prepareAndSchedule(
        make_shared<GetFirstLevelContractorsBalancesTransaction>(
            mNodeUUID,
            command,
            mTrustLines,
            mLog));
}
/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchGetRoutingTablesTransaction(
    RequestRoutingTablesMessage::Shared message) {
    try {
        auto transaction = make_shared<GetRoutingTablesTransaction>(
            mNodeUUID,
            message,
            mTrustLines,
            mStorageHandler,
            mLog);

        prepareAndSchedule(transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchGetRoutingTablesTransaction: "
                "Can't allocate memory for transaction instance.");
    }
}

void TransactionsManager::launchPathsResourcesCollectTransaction(
    const TransactionUUID &requestedTransactionUUID,
    const NodeUUID &destinationNodeUUID) {

    try {
        auto transaction = make_shared<FindPathTransaction>(
            mNodeUUID,
            destinationNodeUUID,
            requestedTransactionUUID,
            mPathsManager,
            mResourcesManager,
            mLog);

        prepareAndSchedule(transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchFindPathTransaction: "
                "Can't allocate memory for transaction instance.");
    }
}

// TODO : should be removed after testing
void TransactionsManager::launchTestCloseCycleTransaction(
    CycleCloserCommand::Shared command) {

    try {
        auto transaction = make_shared<CycleCloserInitiatorTransaction>(
            mNodeUUID,
            command->path(),
            mTrustLines,
            mStorageHandler,
            mMaxFlowCalculationCacheManager,
            mLog);

        prepareAndSchedule(transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchTestCloseCycleTransaction: "
                "Can't allocate memory for transaction instance.");
    }
}

void TransactionsManager::launchTrustLineStatesHandlerTransaction(
    NotificationTrustLineCreatedMessage::Shared message) {
    try {
        auto transaction = make_shared<TrustLineStatesHandlerTransaction>(
            mNodeUUID,
            message->senderUUID,
            message->nodeA,
            message->nodeB,
            TrustLineStatesHandlerTransaction::TrustLineState::Created,
            message->hop,
            mTrustLines,
            mStorageHandler,
            mLog);

        subscribeForSubsidiaryTransactions(transaction->runSubsidiaryTransactionSignal);
        prepareAndSchedule(transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchTrustLineStatesHandlerTransaction: "
                "Can't allocate memory for transaction instance.");
    }
}

void TransactionsManager::launchTrustLineStatesHandlerTransaction(
    NotificationTrustLineRemovedMessage::Shared message) {
    try {
        auto transaction = make_shared<TrustLineStatesHandlerTransaction>(
            mNodeUUID,
            message->senderUUID,
            message->nodeA,
            message->nodeB,
            TrustLineStatesHandlerTransaction::TrustLineState::Removed,
            message->hop,
            mTrustLines,
            mStorageHandler,
            mLog);

        subscribeForSubsidiaryTransactions(transaction->runSubsidiaryTransactionSignal);
        prepareAndSchedule(transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchTrustLineStatesHandlerTransaction: "
                "Can't allocate memory for transaction instance.");
    }
}

void TransactionsManager::launchGetFirstRoutingTableTransaction(
    NeighborsRequestMessage::Shared message)
{
    try {
        auto transaction = make_shared<GetFirstRoutingTableTransaction>(
            mNodeUUID,
            message,
            mTrustLines,
            mLog);

        prepareAndSchedule(transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchGetFirstRoutingTableTransaction: "
                "Can't allocate memory for transaction instance.");
    }
}

void TransactionsManager::attachResourceToTransaction(
    BaseResource::Shared resource) {

    mScheduler->tryAttachResourceToTransaction(
        resource);
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

void TransactionsManager::subscribeForSerializeTransaction(
    TransactionsScheduler::SerializeTransactionSignal &signal) {

    signal.connect(
        boost::bind(
            &TransactionsManager::onSerializeTransaction,
            this,
            _1
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
        50
    );
}

/**
 *
 * @throws bad_alloc;
 */
void TransactionsManager::prepareAndSchedule(
    BaseTransaction::Shared transaction)
{
    subscribeForOutgoingMessages(
        transaction->outgoingMessageIsReadySignal);

    mScheduler->scheduleTransaction(
        transaction);
}
//   ---------------------------------------Cycles part----------------------------------------------

void TransactionsManager::launchThreeNodesCyclesInitTransaction(const NodeUUID &contractorUUID) {
    try {
        auto transaction = make_shared<CyclesThreeNodesInitTransaction>(
            mNodeUUID,
            contractorUUID,
            mTrustLines,
            mStorageHandler,
            mMaxFlowCalculationCacheManager,
            mLog
        );
        subscribeForOutgoingMessages(transaction->outgoingMessageIsReadySignal);
        mScheduler->scheduleTransaction(transaction);
    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchThreeNodesCyclesInitTransaction: "
                "Can't allocate memory for transaction instance.");
    }
}

void TransactionsManager::launchThreeNodesCyclesResponseTransaction(CyclesThreeNodesBalancesRequestMessage::Shared message) {
    try {
        auto transaction = make_shared<CyclesThreeNodesReceiverTransaction>(
            mNodeUUID,
            message,
            mTrustLines,
            mLog
        );
        subscribeForOutgoingMessages(transaction->outgoingMessageIsReadySignal);
        mScheduler->scheduleTransaction(transaction);
    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchThreeNodesCyclesResponseTransaction: "
                "Can't allocate memory for transaction instance.");
    }
}

void TransactionsManager::launchSixNodesCyclesInitTransaction() {

    try {
        auto transaction = make_shared<CyclesSixNodesInitTransaction>(
            mNodeUUID,
            mTrustLines,
            mStorageHandler,
            mMaxFlowCalculationCacheManager,
            mLog
        );
        subscribeForOutgoingMessages(transaction->outgoingMessageIsReadySignal);
        mScheduler->scheduleTransaction(transaction);

    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchSixNodesCyclesInitTransaction: "
                "Can't allocate memory for transaction instance.");
    }
}

void TransactionsManager::launchSixNodesCyclesResponseTransaction(CyclesSixNodesInBetweenMessage::Shared message) {
    try {
        auto transaction = make_shared<CyclesSixNodesReceiverTransaction>(
            mNodeUUID,
            message,
            mTrustLines,
            mLog
        );
        subscribeForOutgoingMessages(transaction->outgoingMessageIsReadySignal);
        mScheduler->scheduleTransaction(transaction);
    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchSixNodesCyclesResponseTransaction: "
                "Can't allocate memory for transaction instance.");
    }
}

void TransactionsManager::launchFiveNodesCyclesInitTransaction() {
    try {
        auto transaction = make_shared<CyclesFiveNodesInitTransaction>(
            mNodeUUID,
            mTrustLines,
            mStorageHandler,
            mMaxFlowCalculationCacheManager,
            mLog
        );
        subscribeForOutgoingMessages(transaction->outgoingMessageIsReadySignal);
        mScheduler->scheduleTransaction(transaction);
    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchFiveNodesCyclesInitTransaction: "
                "Can't allocate memory for transaction instance.");
    }
}

void TransactionsManager::launchFiveNodesCyclesResponseTransaction(CyclesFiveNodesInBetweenMessage::Shared message) {
    try {
        auto transaction = make_shared<CyclesFiveNodesReceiverTransaction>(
            mNodeUUID,
            message,
            mTrustLines,
            mLog
        );
        subscribeForOutgoingMessages(transaction->outgoingMessageIsReadySignal);
        mScheduler->scheduleTransaction(transaction);
    } catch (bad_alloc &) {
        throw MemoryError(
            "TransactionsManager::launchFiveNodesCyclesResponseTransaction: "
                "Can't allocate memory for transaction instance.");
    }
}

void TransactionsManager::launchFourNodesCyclesInitTransaction(const NodeUUID &debtorUUID, const NodeUUID &creditorUUID) {
    try {
        auto transaction = make_shared<CyclesFourNodesInitTransaction>(
                mNodeUUID,
                debtorUUID,
                creditorUUID,
                mTrustLines,
                mStorageHandler,
                mMaxFlowCalculationCacheManager,
                mLog
        );
        subscribeForOutgoingMessages(transaction->outgoingMessageIsReadySignal);
        mScheduler->scheduleTransaction(transaction);
    } catch (bad_alloc &) {
        throw MemoryError(
                "TransactionsManager::launchFourNodesCyclesInitTransaction: "
                        "Can't allocate memory for transaction instance.");
    }
}

void TransactionsManager::launchFourNodesCyclesResponseTransaction(CyclesFourNodesBalancesRequestMessage::Shared message) {
    try {
        auto transaction = make_shared<CyclesFourNodesReceiverTransaction>(
                mNodeUUID,
                message,
                mTrustLines,
                mLog
        );
        subscribeForOutgoingMessages(transaction->outgoingMessageIsReadySignal);
        mScheduler->scheduleTransaction(transaction);
    } catch (bad_alloc &) {
        throw MemoryError(
                "TransactionsManager::launchFourNodesCyclesResponseTransaction: "
                        "Can't allocate memory for transaction instance.");
    }
}

void TransactionsManager::onSerializeTransaction(BaseTransaction::Shared transaction) {
    const auto kTransactionTypeId = transaction->transactionType();
    const auto ioTransaction = mStorageHandler->beginTransaction();
    switch (kTransactionTypeId) {
        case BaseTransaction::TransactionType::CoordinatorPaymentTransaction: {
            const auto kChildTransaction = static_pointer_cast<CoordinatorPaymentTransaction>(transaction);
            const auto transactionBytesAndCount = kChildTransaction->serializeToBytes();
            ioTransaction->transactionHandler()->saveRecord(
                kChildTransaction->currentTransactionUUID(),
                transactionBytesAndCount.first,
                transactionBytesAndCount.second
            );
            break;
        }
        case BaseTransaction::TransactionType::IntermediateNodePaymentTransaction: {
            const auto kChildTransaction = static_pointer_cast<IntermediateNodePaymentTransaction>(transaction);
            const auto transactionBytesAndCount = kChildTransaction->serializeToBytes();
            ioTransaction->transactionHandler()->saveRecord(
                kChildTransaction->currentTransactionUUID(),
                transactionBytesAndCount.first,
                transactionBytesAndCount.second
            );
            break;
        }
        case BaseTransaction::TransactionType::ReceiverPaymentTransaction: {
            const auto kChildTransaction = static_pointer_cast<ReceiverPaymentTransaction>(transaction);
            const auto transactionBytesAndCount = kChildTransaction->serializeToBytes();
            ioTransaction->transactionHandler()->saveRecord(
                kChildTransaction->currentTransactionUUID(),
                transactionBytesAndCount.first,
                transactionBytesAndCount.second
            );
            break;
        }
        default: {
            throw RuntimeError(
                "TrustLinesManager::onSerializeTransaction. "
                    "Unexpected transaction type identifier.");
        }
    }
}
