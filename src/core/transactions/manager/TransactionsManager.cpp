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
    RoutingTableManager *routingTable,
    Logger &logger,
    SubsystemsController *subsystemsController,
    bool iAmGateway) :

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
    mSubsystemsController(subsystemsController),
    mRoutingTable(routingTable),
    mIAmGateway(iAmGateway),

    mScheduler(
        new TransactionsScheduler(
            mIOService,
            mLog)),
    mCyclesManager(
        new CyclesManager(
            mNodeUUID,
            mScheduler.get(),
            mIOService,
            mLog,
            mSubsystemsController))
{
    subscribeForCommandResult(
        mScheduler->commandResultIsReadySignal);
    subscribeForSerializeTransaction(
        mScheduler->serializeTransactionSignal);
    subscribeForCloseCycleTransaction(
        mCyclesManager->closeCycleSignal);
    subscribeForBuildCyclesFiveNodesTransaction(
        mCyclesManager->buildFiveNodesCyclesSignal);
    subscribeForBuildCyclesSixNodesTransaction(
        mCyclesManager->buildSixNodesCyclesSignal);
    subscribeForTryCloseNextCycleSignal(
        mScheduler->cycleCloserTransactionWasFinishedSignal);

    try {
        loadTransactionsFromStorage();

    } catch (exception &e) {
        throw RuntimeError(e.what());
    }

    // todo: send current trust line amount to te contractors
}

void TransactionsManager::loadTransactionsFromStorage()
{
    const auto ioTransaction = mStorageHandler->beginTransaction();
    const auto serializedTAs = ioTransaction->transactionHandler()->allTransactions();

    for(const auto kTABufferAndSize: serializedTAs) {
        BaseTransaction::SerializedTransactionType *transactionType =
                new (kTABufferAndSize.first.get()) BaseTransaction::SerializedTransactionType;
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
                    mPathsManager,
                    mLog,
                    mSubsystemsController);
                subscribeForBuildCyclesThreeNodesTransaction(
                    transaction->mBuildCycleThreeNodesSignal);
                prepareAndSchedule(
                    transaction,
                    true,
                    false,
                    true);
                break;
            }
            case BaseTransaction::TransactionType::IntermediateNodePaymentTransaction: {
                auto transaction = make_shared<IntermediateNodePaymentTransaction>(
                    kTABufferAndSize.first,
                    mNodeUUID,
                    mTrustLines,
                    mStorageHandler,
                    mMaxFlowCalculationCacheManager,
                    mLog,
                    mSubsystemsController);
                subscribeForBuildCyclesThreeNodesTransaction(
                    transaction->mBuildCycleThreeNodesSignal);
                subscribeForBuildCyclesFourNodesTransaction(
                    transaction->mBuildCycleFourNodesSignal);
                prepareAndSchedule(
                    transaction,
                    false,
                    false,
                    true);
                break;
            }
            case BaseTransaction::TransactionType::ReceiverPaymentTransaction: {
                auto transaction = make_shared<ReceiverPaymentTransaction>(
                    kTABufferAndSize.first,
                    mNodeUUID,
                    mTrustLines,
                    mStorageHandler,
                    mMaxFlowCalculationCacheManager,
                    mLog,
                    mSubsystemsController);
                subscribeForBuildCyclesThreeNodesTransaction(
                    transaction->mBuildCycleThreeNodesSignal);
                prepareAndSchedule(
                    transaction,
                    false,
                    false,
                    true);
                break;
            }
            case BaseTransaction::TransactionType::Payments_CycleCloserInitiatorTransaction: {
                auto transaction = make_shared<CycleCloserInitiatorTransaction>(
                    kTABufferAndSize.first,
                    mNodeUUID,
                    mTrustLines,
                    mCyclesManager.get(),
                    mStorageHandler,
                    mMaxFlowCalculationCacheManager,
                    mLog,
                    mSubsystemsController);
                prepareAndSchedule(
                    transaction,
                    false,
                    false,
                    true);
                break;
            }
            case BaseTransaction::TransactionType::Payments_CycleCloserIntermediateNodeTransaction: {
                auto transaction = make_shared<CycleCloserIntermediateNodeTransaction>(
                    kTABufferAndSize.first,
                    mNodeUUID,
                    mTrustLines,
                    mCyclesManager.get(),
                    mStorageHandler,
                    mMaxFlowCalculationCacheManager,
                    mLog,
                    mSubsystemsController);
                prepareAndSchedule(
                    transaction,
                    false,
                    false,
                    true);
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

void TransactionsManager::processCommand(
    BaseUserCommand::Shared command)
{
    // ToDo: sort calls in the call probabilty order.
    // For example, max flows calculations would be called much ofetn, then credit usage transactions.
    // So, why we are checking trust lines commands first, and max flow is only in the middle of the check sequence?

    if (command->identifier() == SetOutgoingTrustLineCommand::identifier()) {
        launchSetOutgoingTrustLineTransaction(
            static_pointer_cast<SetOutgoingTrustLineCommand>(
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

    } else if (command->identifier() == HistoryWithContractorCommand::identifier()){
        launchHistoryWithContractorTransaction(
            static_pointer_cast<HistoryWithContractorCommand>(
                command));

    } else if (command->identifier() == GetFirstLevelContractorsCommand::identifier()){
        launchGetFirstLevelContractorsTransaction(
                static_pointer_cast<GetFirstLevelContractorsCommand>(
                        command));

    } else if (command->identifier() == GetTrustLinesCommand::identifier()){
        launchGetTrustlinesTransaction(
            static_pointer_cast<GetTrustLinesCommand>(
                command));

    } else if (command->identifier() == GetTrustLineCommand::identifier()){
        launchGetTrustlineTransaction(
            static_pointer_cast<GetTrustLineCommand>(
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
    // ToDo: sort calls in the call probability order.
    // For example, max flows calculations would be called much oftetn, then credit usage transactions.

    /*
     * Max flow
     */
    if (message->typeID() == Message::MessageType::MaxFlow_InitiateCalculation) {
        launchReceiveMaxFlowCalculationOnTargetTransaction(
            static_pointer_cast<InitiateMaxFlowCalculationMessage>(message));

    } else if (message->typeID() == Message::MessageType::MaxFlow_ResultMaxFlowCalculation) {
        launchReceiveResultMaxFlowCalculationTransaction(
            static_pointer_cast<ResultMaxFlowCalculationMessage>(message));

    } else if (message->typeID() == Message::MessageType::MaxFlow_ResultMaxFlowCalculationFromGateway) {
        launchReceiveResultMaxFlowCalculationTransactionFromGateway(
            static_pointer_cast<ResultMaxFlowCalculationGatewayMessage>(message));

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

    } else if (message->typeID() == Message::Payments_IntermediateNodeCycleReservationRequest) {
        // It is possible, that transaction was already initialised
        // by the ReceiverInitPaymentRequest.
        // In this case - message must be simply attached to it,
        // no new transaction must be launched.
        try {
            mScheduler->tryAttachMessageToTransaction(message);

        } catch (NotFoundError &) {
            launchCycleCloserIntermediateNodeTransaction(
                static_pointer_cast<IntermediateNodeCycleReservationRequestMessage>(message));
        }
    } else if(message->typeID() == Message::MessageType::Payments_VotesStatusRequest){
        launchVotesResponsePaymentsTransaction(
            static_pointer_cast<VotesStatusRequestMessage>(message));

    /*
     * Cycles
     */
    } else if (message->typeID() == Message::MessageType::Cycles_SixNodesMiddleware) {
        launchSixNodesCyclesResponseTransaction(
            static_pointer_cast<CyclesSixNodesInBetweenMessage>(message));

    } else if (message->typeID() == Message::MessageType::Cycles_FiveNodesMiddleware) {
        launchFiveNodesCyclesResponseTransaction(
            static_pointer_cast<CyclesFiveNodesInBetweenMessage>(message));

    } else if (message->typeID() == Message::MessageType::Cycles_ThreeNodesBalancesRequest){
        launchThreeNodesCyclesResponseTransaction(
            static_pointer_cast<CyclesThreeNodesBalancesRequestMessage>(message));

    } else if (message->typeID() == Message::MessageType::Cycles_FourNodesBalancesRequest){
        launchFourNodesCyclesResponseTransaction(
            static_pointer_cast<CyclesFourNodesBalancesRequestMessage>(message));

    /*
     * Trust lines
     */
    } else if (message->typeID() == Message::TrustLines_SetIncoming) {
        launchSetIncomingTrustLineTransaction(
            static_pointer_cast<SetIncomingTrustLineMessage>(message));

    /*
     * RoutingTable
    */
    } else if (message->typeID() == Message::RoutingTableRequest) {
        launchRoutingTableResponseTransaction(
            static_pointer_cast<RoutingTableRequestMessage>(message));
    /*
     * Gateway notification
     */
    } else if (message->typeID() == Message::GatewayNotification) {
        launchGatewayNotificationReceiverTransaction(
            static_pointer_cast<GatewayNotificationMessage>(message));

    } else {
        mScheduler->tryAttachMessageToTransaction(message);
    }
}

void TransactionsManager::launchSetOutgoingTrustLineTransaction(
    SetOutgoingTrustLineCommand::Shared command)
{
    prepareAndSchedule(
        make_shared<SetOutgoingTrustLineTransaction>(
            mNodeUUID,
            command,
            mTrustLines,
            mStorageHandler,
            mMaxFlowCalculationCacheManager,
            mSubsystemsController,
            mLog),
        true,
        false,
        true);
}

void TransactionsManager::launchSetIncomingTrustLineTransaction(
    SetIncomingTrustLineMessage::Shared message)
{
    prepareAndSchedule(
        make_shared<SetIncomingTrustLineTransaction>(
            mNodeUUID,
            message,
            mTrustLines,
            mStorageHandler,
            mMaxFlowCalculationCacheManager,
            mIAmGateway,
            mLog),
        true,
        false,
        true);
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchInitiateMaxFlowCalculatingTransaction(
    InitiateMaxFlowCalculationCommand::Shared command) {

    try {
        prepareAndSchedule(
            make_shared<InitiateMaxFlowCalculationTransaction>(
                mNodeUUID,
                command,
                mTrustLines,
                mMaxFlowCalculationTrustLineManager,
                mMaxFlowCalculationCacheManager,
                mLog),
            true,
            true,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchReceiveMaxFlowCalculationOnTargetTransaction(
    InitiateMaxFlowCalculationMessage::Shared message) {

    try {
        prepareAndSchedule(
            make_shared<ReceiveMaxFlowCalculationOnTargetTransaction>(
                mNodeUUID,
                message,
                mTrustLines,
                mMaxFlowCalculationCacheManager,
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchReceiveResultMaxFlowCalculationTransaction(
    ResultMaxFlowCalculationMessage::Shared message) {

    try {
        prepareAndSchedule(
            make_shared<ReceiveResultMaxFlowCalculationTransaction>(
                mNodeUUID,
                message,
                mTrustLines,
                mMaxFlowCalculationTrustLineManager,
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchReceiveResultMaxFlowCalculationTransactionFromGateway(
    ResultMaxFlowCalculationGatewayMessage::Shared message) {

    try {
        prepareAndSchedule(
            make_shared<ReceiveResultMaxFlowCalculationTransaction>(
                mNodeUUID,
                message,
                mTrustLines,
                mMaxFlowCalculationTrustLineManager,
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchMaxFlowCalculationSourceFstLevelTransaction(
    MaxFlowCalculationSourceFstLevelMessage::Shared message) {

    try {
        prepareAndSchedule(
            make_shared<MaxFlowCalculationSourceFstLevelTransaction>(
                mNodeUUID,
                message,
                mTrustLines,
                mMaxFlowCalculationCacheManager,
                mLog,
                mIAmGateway),
            false,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchMaxFlowCalculationTargetFstLevelTransaction(
    MaxFlowCalculationTargetFstLevelMessage::Shared message) {

    try {
        prepareAndSchedule(
            make_shared<MaxFlowCalculationTargetFstLevelTransaction>(
                mNodeUUID,
                message,
                mTrustLines,
                mMaxFlowCalculationCacheManager,
                mLog,
                mIAmGateway),
            false,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchMaxFlowCalculationSourceSndLevelTransaction(
    MaxFlowCalculationSourceSndLevelMessage::Shared message) {

    try {
        prepareAndSchedule(
            make_shared<MaxFlowCalculationSourceSndLevelTransaction>(
                mNodeUUID,
                message,
                mTrustLines,
                mMaxFlowCalculationCacheManager,
                mLog,
                mIAmGateway),
            false,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchMaxFlowCalculationTargetSndLevelTransaction(
    MaxFlowCalculationTargetSndLevelMessage::Shared message) {

    try {
        prepareAndSchedule(
            make_shared<MaxFlowCalculationTargetSndLevelTransaction>(
                mNodeUUID,
                message,
                mTrustLines,
                mMaxFlowCalculationCacheManager,
                mLog,
                mIAmGateway),
            false,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }
}

void TransactionsManager::launchCoordinatorPaymentTransaction(
    CreditUsageCommand::Shared command)
{
    auto transaction = make_shared<CoordinatorPaymentTransaction>(
        mNodeUUID,
        command,
        mTrustLines,
        mStorageHandler,
        mMaxFlowCalculationCacheManager,
        mResourcesManager,
        mPathsManager,
        mLog,
        mSubsystemsController);
    subscribeForBuildCyclesThreeNodesTransaction(
        transaction->mBuildCycleThreeNodesSignal);
    prepareAndSchedule(transaction, true, false, true);
}

void TransactionsManager::launchReceiverPaymentTransaction(
    ReceiverInitPaymentRequestMessage::Shared message)
{
    auto transaction = make_shared<ReceiverPaymentTransaction>(
        mNodeUUID,
        message,
        mTrustLines,
        mStorageHandler,
        mMaxFlowCalculationCacheManager,
        mLog,
        mSubsystemsController);
    subscribeForBuildCyclesThreeNodesTransaction(
        transaction->mBuildCycleThreeNodesSignal);
    prepareAndSchedule(transaction, false, false, true);
}

void TransactionsManager::launchIntermediateNodePaymentTransaction(
    IntermediateNodeReservationRequestMessage::Shared message)
{
    auto transaction = make_shared<IntermediateNodePaymentTransaction>(
        mNodeUUID,
        message,
        mTrustLines,
        mStorageHandler,
        mMaxFlowCalculationCacheManager,
        mLog,
        mSubsystemsController);
    subscribeForBuildCyclesThreeNodesTransaction(
        transaction->mBuildCycleThreeNodesSignal);
    subscribeForBuildCyclesFourNodesTransaction(
        transaction->mBuildCycleFourNodesSignal);
    prepareAndSchedule(transaction, false, false, true);
}

void TransactionsManager::launchCycleCloserIntermediateNodeTransaction(
    IntermediateNodeCycleReservationRequestMessage::Shared message)
{
    try{
        prepareAndSchedule(
            make_shared<CycleCloserIntermediateNodeTransaction>(
                mNodeUUID,
                message,
                mTrustLines,
                mCyclesManager.get(),
                mStorageHandler,
                mMaxFlowCalculationCacheManager,
                mLog,
                mSubsystemsController),
            false,
            false,
            true
        );
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }

}

void TransactionsManager::launchVotesResponsePaymentsTransaction(
    VotesStatusRequestMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<VotesStatusResponsePaymentTransaction>(
                mNodeUUID,
                message,
                mStorageHandler,
                mScheduler->isTransactionInProcess(
                    message->transactionUUID()),
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchTotalBalancesTransaction(
    TotalBalancesCommand::Shared command) {

    try {
        prepareAndSchedule(
            make_shared<TotalBalancesTransaction>(
                mNodeUUID,
                command,
                mTrustLines,
                mLog),
            true,
            false,
            true
        );
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchTotalBalancesTransaction(
    InitiateTotalBalancesMessage::Shared message) {

    try {
        prepareAndSchedule(
            make_shared<TotalBalancesTransaction>(
                mNodeUUID,
                message,
                mTrustLines,
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchTotalBalancesRemoteNodeTransaction(
    TotalBalancesRemouteNodeCommand::Shared command) {

    try {
        prepareAndSchedule(
            make_shared<TotalBalancesFromRemoutNodeTransaction>(
                mNodeUUID,
                command,
                mLog),
            true,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchHistoryPaymentsTransaction(
    HistoryPaymentsCommand::Shared command) {
    try {
        prepareAndSchedule(
            make_shared<HistoryPaymentsTransaction>(
                mNodeUUID,
                command,
                mStorageHandler,
                mLog),
            true,
            false,
            false);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchHistoryTrustLinesTransaction(
    HistoryTrustLinesCommand::Shared command) {
    try {
        prepareAndSchedule(
            make_shared<HistoryTrustLinesTransaction>(
                mNodeUUID,
                command,
                mStorageHandler,
                mLog),
            true,
            false,
            false);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchHistoryWithContractorTransaction(
    HistoryWithContractorCommand::Shared command) {
    try {
        prepareAndSchedule(
            make_shared<HistoryWithContractorTransaction>(
                mNodeUUID,
                command,
                mStorageHandler,
                mLog),
            true,
            false,
            false);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }
}

void TransactionsManager::launchGetFirstLevelContractorsTransaction(
    GetFirstLevelContractorsCommand::Shared command)
{
    try {
        prepareAndSchedule(
            make_shared<GetFirstLevelContractorsTransaction>(
                mNodeUUID,
                command,
                mTrustLines,
                mLog),
            true,
            false,
            false);
    } catch (ValueError &e){
        throw ValueError(e.message());
    }
}


void TransactionsManager::launchGetTrustlinesTransaction(
    GetTrustLinesCommand::Shared command)
{
    try {
        prepareAndSchedule(
            make_shared<GetFirstLevelContractorsBalancesTransaction>(
                mNodeUUID,
                command,
                mTrustLines,
                mLog),
            true,
            false,
            false);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }

}

void TransactionsManager::launchGetTrustlineTransaction(
    GetTrustLineCommand::Shared command)
{
    try {
        prepareAndSchedule(
            make_shared<GetFirstLevelContractorBalanceTransaction>(
                mNodeUUID,
                command,
                mTrustLines,
                mLog),
            true,
            false,
            false);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }

}

void TransactionsManager::launchFindPathByMaxFlowTransaction(
    const TransactionUUID &requestedTransactionUUID,
    const NodeUUID &destinationNodeUUID)
{
    try {
        prepareAndSchedule(
            make_shared<FindPathByMaxFlowTransaction>(
                mNodeUUID,
                destinationNodeUUID,
                requestedTransactionUUID,
                mPathsManager,
                mResourcesManager,
                mTrustLines,
                mMaxFlowCalculationTrustLineManager,
                mMaxFlowCalculationCacheManager,
                mLog),
            true,
            true,
            false);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    }
}

void TransactionsManager::launchGatewayNotificationSenderTransaction()
{
    try {
        prepareAndSchedule(
            make_shared<GatewayNotificationSenderTransaction>(
                mNodeUUID,
                mTrustLines,
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    }
}

void TransactionsManager::launchGatewayNotificationReceiverTransaction(
    GatewayNotificationMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<GatewayNotificationReceiverTransaction>(
                mNodeUUID,
                message,
                mTrustLines,
                mStorageHandler,
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
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

    // ToDo: connect signals of transaction and core dirctly (signal -> signal)
    // Boost allows this type of connectivity.
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

void TransactionsManager::subscribeForBuildCyclesThreeNodesTransaction(
    BasePaymentTransaction::BuildCycleThreeNodesSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onBuidCycleThreeNodesTransaction,
            this,
            _1));
}

void TransactionsManager::subscribeForBuildCyclesFourNodesTransaction(
    BasePaymentTransaction::BuildCycleFourNodesSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onBuildCycleFourNodesTransaction,
            this,
            _1));
}

void TransactionsManager::subscribeForBuildCyclesFiveNodesTransaction(
    CyclesManager::BuildFiveNodesCyclesSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onBuidCycleFiveNodesTransaction,
            this));
}

void TransactionsManager::subscribeForBuildCyclesSixNodesTransaction(
    CyclesManager::BuildSixNodesCyclesSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onBuidCycleSixNodesTransaction,
            this));
}

void TransactionsManager::subscribeForCloseCycleTransaction(
    CyclesManager::CloseCycleSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onCloseCycleTransaction,
            this,
            _1));
}

void TransactionsManager::subscribeForTryCloseNextCycleSignal(
    TransactionsScheduler::CycleCloserTransactionWasFinishedSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onTryCloseNextCycleSlot,
            this));
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

        mLog.info("Result for command " + result->identifier());
        mLog.logSuccess(
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

void TransactionsManager::onBuidCycleThreeNodesTransaction(
    vector<NodeUUID> &contractorsUUID)
{
    for (const auto &contractorUUID : contractorsUUID) {
        launchThreeNodesCyclesInitTransaction(
            contractorUUID);
    }
}

void TransactionsManager::onBuildCycleFourNodesTransaction(
    vector<NodeUUID> &creditors)
{
    for (const auto &kCreditor : creditors) {
        launchFourNodesCyclesInitTransaction(
            kCreditor);
    }
}

void TransactionsManager::onBuidCycleFiveNodesTransaction()
{
    launchFiveNodesCyclesInitTransaction();
}

void TransactionsManager::onBuidCycleSixNodesTransaction()
{
    launchSixNodesCyclesInitTransaction();
}

void TransactionsManager::onCloseCycleTransaction(
    Path::ConstShared cycle)
{
    try {
        prepareAndSchedule(
            make_shared<CycleCloserInitiatorTransaction>(
                mNodeUUID,
                cycle,
                mTrustLines,
                mCyclesManager.get(),
                mStorageHandler,
                mMaxFlowCalculationCacheManager,
                mLog,
                mSubsystemsController),
            true,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }
}

void TransactionsManager::onTryCloseNextCycleSlot()
{
    mCyclesManager->closeOneCycle(true);
}

/**
 *
 * @throws bad_alloc;
 */
void TransactionsManager::prepareAndSchedule(
    BaseTransaction::Shared transaction,
    bool regenerateUUID,
    bool subsidiaryTransactionSubscribe,
    bool outgoingMessagesSubscribe)
{
    if (outgoingMessagesSubscribe)
        subscribeForOutgoingMessages(
            transaction->outgoingMessageIsReadySignal);
    if (subsidiaryTransactionSubscribe)
        subscribeForSubsidiaryTransactions(
            transaction->runSubsidiaryTransactionSignal);

    while (true) {
        try {

            mScheduler->scheduleTransaction(
                transaction);
            return;

        } catch(bad_alloc &) {
            throw bad_alloc();

        } catch(ConflictError &e) {
            mLog.warning("prepareAndSchedule:") << "TransactionUUID: " << transaction->currentTransactionUUID()
                                              << " New TransactionType:" << transaction->transactionType()
                                              << " Recreate.";
            if (regenerateUUID) {
                transaction->recreateTransactionUUID();
            } else {
                mLog.warning("prepareAndSchedule:") << "TransactionUUID: " << transaction->currentTransactionUUID()
                                                  << " New TransactionType:" << transaction->transactionType()
                                                  << "Exit.";
                return;
            }
        }
    }
}

void TransactionsManager::launchThreeNodesCyclesInitTransaction(
    const NodeUUID &contractorUUID)
{
    try {
        prepareAndSchedule(
            make_shared<CyclesThreeNodesInitTransaction>(
                mNodeUUID,
                contractorUUID,
                mTrustLines,
                mRoutingTable,
                mCyclesManager.get(),
                mStorageHandler,
                mLog),
            true,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    }

}

void TransactionsManager::launchThreeNodesCyclesResponseTransaction(
    CyclesThreeNodesBalancesRequestMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<CyclesThreeNodesReceiverTransaction>(
                mNodeUUID,
                message,
                mTrustLines,
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    }
}

void TransactionsManager::launchSixNodesCyclesInitTransaction()
{
    try {
        prepareAndSchedule(
            make_shared<CyclesSixNodesInitTransaction>(
                mNodeUUID,
                mTrustLines,
                mCyclesManager.get(),
                mStorageHandler,
                mLog),
            true,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    }
}

void TransactionsManager::launchSixNodesCyclesResponseTransaction(
    CyclesSixNodesInBetweenMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<CyclesSixNodesReceiverTransaction>(
                mNodeUUID,
                message,
                mTrustLines,
                mLog),
            false,
            false,
            true
        );
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    }
}

void TransactionsManager::launchFiveNodesCyclesInitTransaction()
{
    try {
        prepareAndSchedule(
            make_shared<CyclesFiveNodesInitTransaction>(
                mNodeUUID,
                mTrustLines,
                mCyclesManager.get(),
                mStorageHandler,
                mLog),
            true,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    }
}

void TransactionsManager::launchFiveNodesCyclesResponseTransaction(
    CyclesFiveNodesInBetweenMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<CyclesFiveNodesReceiverTransaction>(
                mNodeUUID,
                message,
                mTrustLines,
                mLog),
            false,
            false,
            true
        );
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    }
}

void TransactionsManager::launchFourNodesCyclesInitTransaction(
    const NodeUUID &creditorUUID)
{
    try {
        prepareAndSchedule(
            make_shared<CyclesFourNodesInitTransaction>(
                mNodeUUID,
                creditorUUID,
                mTrustLines,
                mRoutingTable,
                mCyclesManager.get(),
                mStorageHandler,
                mLog),
            true,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    }
}

void TransactionsManager::launchFourNodesCyclesResponseTransaction(
    CyclesFourNodesBalancesRequestMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<CyclesFourNodesReceiverTransaction>(
                mNodeUUID,
                message,
                mTrustLines,
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    }
}

void TransactionsManager::onSerializeTransaction(
    BaseTransaction::Shared transaction)
{
    const auto kTransactionTypeId = transaction->transactionType();
    const auto ioTransaction = mStorageHandler->beginTransaction();
    switch (kTransactionTypeId) {
        case BaseTransaction::TransactionType::CoordinatorPaymentTransaction: {
            const auto kChildTransaction = static_pointer_cast<CoordinatorPaymentTransaction>(transaction);
            const auto transactionBytesAndCount = kChildTransaction->serializeToBytes();
            ioTransaction->transactionHandler()->saveRecord(
                kChildTransaction->currentTransactionUUID(),
                transactionBytesAndCount.first,
                transactionBytesAndCount.second);
            break;
        }
        case BaseTransaction::TransactionType::IntermediateNodePaymentTransaction: {
            const auto kChildTransaction = static_pointer_cast<IntermediateNodePaymentTransaction>(transaction);
            const auto transactionBytesAndCount = kChildTransaction->serializeToBytes();
            ioTransaction->transactionHandler()->saveRecord(
                kChildTransaction->currentTransactionUUID(),
                transactionBytesAndCount.first,
                transactionBytesAndCount.second);
            break;
        }
        case BaseTransaction::TransactionType::ReceiverPaymentTransaction: {
            const auto kChildTransaction = static_pointer_cast<ReceiverPaymentTransaction>(transaction);
            const auto transactionBytesAndCount = kChildTransaction->serializeToBytes();
            ioTransaction->transactionHandler()->saveRecord(
                kChildTransaction->currentTransactionUUID(),
                transactionBytesAndCount.first,
                transactionBytesAndCount.second);
            break;
        }
        case BaseTransaction::TransactionType::Payments_CycleCloserInitiatorTransaction: {
            const auto kChildTransaction = static_pointer_cast<CycleCloserInitiatorTransaction>(transaction);
            const auto transactionBytesAndCount = kChildTransaction->serializeToBytes();
            ioTransaction->transactionHandler()->saveRecord(
                kChildTransaction->currentTransactionUUID(),
                transactionBytesAndCount.first,
                transactionBytesAndCount.second);
            break;
        }
        case BaseTransaction::TransactionType::Payments_CycleCloserIntermediateNodeTransaction: {
            const auto kChildTransaction = static_pointer_cast<CycleCloserIntermediateNodeTransaction>(transaction);
            const auto transactionBytesAndCount = kChildTransaction->serializeToBytes();
            ioTransaction->transactionHandler()->saveRecord(
                kChildTransaction->currentTransactionUUID(),
                transactionBytesAndCount.first,
                transactionBytesAndCount.second);
            break;
        }
        default: {
            throw RuntimeError(
                "TrustLinesManager::onSerializeTransaction. "
                    "Unexpected transaction type identifier.");
        }
    }
}

void TransactionsManager::launchRoutingTableResponseTransaction(
    RoutingTableRequestMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<RoutingTableResponseTransaction>(
                mNodeUUID,
                message,
                mTrustLines,
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    }
}

void TransactionsManager::launchRoutingTableRequestTransaction()
{
    try {
        prepareAndSchedule(
            make_shared<RoutingTableInitTransaction>(
                mNodeUUID,
                mTrustLines,
                mRoutingTable,
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    }
}

#ifdef TESTS
void TransactionsManager::setMeAsGateway()
{
    mIAmGateway = true;
}
#endif
