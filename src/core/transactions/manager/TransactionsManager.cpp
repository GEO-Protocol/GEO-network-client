#include "TransactionsManager.h"

/*!
 *
 * Throws RuntimeError in case if some internal components can't be initialised.
 */
TransactionsManager::TransactionsManager(
    NodeUUID &nodeUUID,
    as::io_service &IOService,
    ContractorsManager *contractorsManager,
    EquivalentsSubsystemsRouter *equivalentsSubsystemsRouter,
    ResourcesManager *resourcesManager,
    ResultsInterface *resultsInterface,
    StorageHandler *storageHandler,
    Keystore *keystore,
    Logger &logger,
    SubsystemsController *subsystemsController,
    TrustLinesInfluenceController *trustLinesInfluenceController) :

    mNodeUUID(nodeUUID),
    mIOService(IOService),
    mContractorsManager(contractorsManager),
    mEquivalentsSubsystemsRouter(equivalentsSubsystemsRouter),
    mResourcesManager(resourcesManager),
    mResultsInterface(resultsInterface),
    mStorageHandler(storageHandler),
    mKeysStore(keystore),
    mLog(logger),
    mSubsystemsController(subsystemsController),
    mTrustLinesInfluenceController(trustLinesInfluenceController),

    mScheduler(
        new TransactionsScheduler(
            mIOService,
            mTrustLinesInfluenceController,
            mLog)),

    mEquivalentsCyclesSubsystemsRouter(
        new EquivalentsCyclesSubsystemsRouter(
            mNodeUUID,
            mScheduler.get(),
            mSubsystemsController,
            mIOService,
            mEquivalentsSubsystemsRouter->equivalents(),
            mLog))
{
    subscribeForCommandResult(
        mScheduler->commandResultIsReadySignal);
    subscribeForSerializeTransaction(
        mScheduler->serializeTransactionSignal);
    subscribeForCloseCycleTransaction(
        mEquivalentsCyclesSubsystemsRouter->closeCycleSignal);
    subscribeForBuildCyclesFiveNodesTransaction(
        mEquivalentsCyclesSubsystemsRouter->buildFiveNodesCyclesSignal);
    subscribeForBuildCyclesSixNodesTransaction(
        mEquivalentsCyclesSubsystemsRouter->buildSixNodesCyclesSignal);
    subscribeForTryCloseNextCycleSignal(
        mScheduler->cycleCloserTransactionWasFinishedSignal);
    subscribeForGatewayNotificationSignal(
        mEquivalentsSubsystemsRouter->gatewayNotificationSignal);

    try {
        loadTransactionsFromStorage();

    } catch (exception &e) {
        error() << "loadTransactionsFromStorage. Details: " << e.what();
        throw RuntimeError(e.what());
    }
}

void TransactionsManager::loadTransactionsFromStorage()
{
    const auto ioTransaction = mStorageHandler->beginTransaction();
    const auto serializedTAs = ioTransaction->transactionHandler()->allTransactions();

    for(const auto &kTABuffer: serializedTAs) {
        auto *transactionType =
                new (kTABuffer.get()) BaseTransaction::SerializedTransactionType;
        auto transactionTypeId = *transactionType;
        auto *equivalent =
                new (kTABuffer.get()
                     + sizeof(BaseTransaction::SerializedTransactionType)) SerializedEquivalent;

        switch (transactionTypeId) {
            case BaseTransaction::IntermediateNodePaymentTransaction: {
                try {
                    auto transaction = make_shared<IntermediateNodePaymentTransaction>(
                        kTABuffer,
                        mNodeUUID,
                        mEquivalentsSubsystemsRouter->iAmGateway(*equivalent),
                        mContractorsManager,
                        mEquivalentsSubsystemsRouter->trustLinesManager(*equivalent),
                        mStorageHandler,
                        mEquivalentsSubsystemsRouter->topologyCacheManager(*equivalent),
                        mEquivalentsSubsystemsRouter->maxFlowCacheManager(*equivalent),
                        mKeysStore,
                        mLog,
                        mSubsystemsController);
                    subscribeForBuildCyclesThreeNodesTransaction(
                        transaction->mBuildCycleThreeNodesSignal);
                    subscribeForBuildCyclesFourNodesTransaction(
                        transaction->mBuildCycleFourNodesSignal);
                    subscribeForTrustLineActionSignal(
                        transaction->trustLineActionSignal);
                    prepareAndSchedule(
                        transaction,
                        false,
                        false,
                        true);
                } catch (NotFoundError &e) {
                    error() << "There are no subsystems for serialized IntermediateNodePaymentTransaction "
                            "with equivalent " << *equivalent << " Details are: " << e.what();
                    continue;
                }
                break;
            }
            case BaseTransaction::ReceiverPaymentTransaction: {
                try {
                    auto transaction = make_shared<ReceiverPaymentTransaction>(
                        kTABuffer,
                        mNodeUUID,
                        mEquivalentsSubsystemsRouter->iAmGateway(*equivalent),
                        mContractorsManager,
                        mEquivalentsSubsystemsRouter->trustLinesManager(*equivalent),
                        mStorageHandler,
                        mEquivalentsSubsystemsRouter->topologyCacheManager(*equivalent),
                        mEquivalentsSubsystemsRouter->maxFlowCacheManager(*equivalent),
                        mKeysStore,
                        mLog,
                        mSubsystemsController);
                    subscribeForBuildCyclesThreeNodesTransaction(
                        transaction->mBuildCycleThreeNodesSignal);
                    subscribeForBuildCyclesFourNodesTransaction(
                        transaction->mBuildCycleFourNodesSignal);
                    subscribeForTrustLineActionSignal(
                        transaction->trustLineActionSignal);
                    prepareAndSchedule(
                        transaction,
                        false,
                        false,
                        true);
                } catch (NotFoundError &e) {
                    error() << "There are no subsystems for serialized ReceiverPaymentTransaction "
                            "with equivalent " << *equivalent << " Details are: " << e.what();
                    continue;
                }
                break;
            }
            case BaseTransaction::Payments_CycleCloserIntermediateNodeTransaction: {
                try {
                    auto transaction = make_shared<CycleCloserIntermediateNodeTransaction>(
                        kTABuffer,
                        mNodeUUID,
                        mContractorsManager,
                        mEquivalentsSubsystemsRouter->trustLinesManager(*equivalent),
                        mEquivalentsCyclesSubsystemsRouter->cyclesManager(*equivalent),
                        mStorageHandler,
                        mEquivalentsSubsystemsRouter->topologyCacheManager(*equivalent),
                        mEquivalentsSubsystemsRouter->maxFlowCacheManager(*equivalent),
                        mKeysStore,
                        mLog,
                        mSubsystemsController);
                    subscribeForTrustLineActionSignal(
                        transaction->trustLineActionSignal);
                    prepareAndSchedule(
                        transaction,
                        false,
                        false,
                        true);
                } catch (NotFoundError &e) {
                    error() << "There are no subsystems for serialized CycleCloserIntermediateNodeTransaction "
                            "with equivalent " << *equivalent << " Details are: " << e.what();
                    continue;
                }
                break;
            }
            case BaseTransaction::ConflictResolverInitiatorTransactionType: {
                try {
                    auto transaction = make_shared<ConflictResolverInitiatorTransaction>(
                        kTABuffer,
                        mNodeUUID,
                        mContractorsManager,
                        mEquivalentsSubsystemsRouter->trustLinesManager(*equivalent),
                        mStorageHandler,
                        mKeysStore,
                        mTrustLinesInfluenceController,
                        mLog);
                    subscribeForProcessingConfirmationMessage(
                        transaction->processConfirmationMessageSignal);
                    prepareAndSchedule(
                        transaction,
                        false,
                        true,
                        true);
                } catch (NotFoundError &e) {
                    error() << "There are no subsystems for serialized ConflictResolverInitiatorTransaction "
                            "with equivalent " << *equivalent << " Details are: " << e.what();
                    continue;
                }
                break;
            }
            default: {
                throw RuntimeError(
                    "TrustLinesManager::loadTransactions. "
                        "Unexpected transaction type identifier " + to_string(transactionTypeId));
            }
        }

    }
}

void TransactionsManager::processCommand(
    BaseUserCommand::Shared command)
{
    // ToDo: sort calls in the call probability order.
    // For example, max flows calculations would be called much often, then credit usage transactions.
    // So, why we are checking trust lines commands first, and max flow is only in the middle of the check sequence?
    if (command->identifier() == InitTrustLineCommand::identifier()) {
        launchInitTrustLineTransaction(
            static_pointer_cast<InitTrustLineCommand>(
                command));

    } else if (command->identifier() == SetOutgoingTrustLineCommand::identifier()) {
        launchSetOutgoingTrustLineTransaction(
            static_pointer_cast<SetOutgoingTrustLineCommand>(
                command));

    } else if (command->identifier() == CloseIncomingTrustLineCommand::identifier()) {
        launchCloseIncomingTrustLineTransaction(
            static_pointer_cast<CloseIncomingTrustLineCommand>(
                command));

    } else if (command->identifier() == ShareKeysCommand::identifier()) {
        launchPublicKeysSharingSourceTransaction(
            static_pointer_cast<ShareKeysCommand>(
                command));

    } else if (command->identifier() == CreditUsageCommand::identifier()) {
        launchCoordinatorPaymentTransaction(
            dynamic_pointer_cast<CreditUsageCommand>(
                command));

    } else if (command->identifier() == InitiateMaxFlowCalculationCommand::identifier()){
        launchInitiateMaxFlowCalculatingTransaction(
            static_pointer_cast<InitiateMaxFlowCalculationCommand>(
                command));

    } else if (command->identifier() == InitiateMaxFlowCalculationFullyCommand::identifier()){
        launchMaxFlowCalculationFullyTransaction(
            static_pointer_cast<InitiateMaxFlowCalculationFullyCommand>(
                command));

    } else if (command->identifier() == TotalBalancesCommand::identifier()){
        launchTotalBalancesTransaction(
            static_pointer_cast<TotalBalancesCommand>(
                command));

    } else if (command->identifier() == HistoryPaymentsCommand::identifier()){
        launchHistoryPaymentsTransaction(
            static_pointer_cast<HistoryPaymentsCommand>(
                command));

    } else if (command->identifier() == HistoryAdditionalPaymentsCommand::identifier()){
        launchAdditionalHistoryPaymentsTransaction(
            static_pointer_cast<HistoryAdditionalPaymentsCommand>(
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
        launchGetTrustLinesTransaction(
            static_pointer_cast<GetTrustLinesCommand>(
                command));

    } else if (command->identifier() == GetTrustLineCommand::identifier()){
        launchGetTrustLineTransaction(
            static_pointer_cast<GetTrustLineCommand>(
                command));

    } else if (command->identifier() == EquivalentListCommand::identifier()){
        launchGetEquivalentListTransaction(
            static_pointer_cast<EquivalentListCommand>(
                command));

    // BlackList Commands
    } else if (command->identifier() == AddNodeToBlackListCommand::identifier()){

        launchAddNodeToBlackListTransaction(
            static_pointer_cast<AddNodeToBlackListCommand>(
                command));

    } else if (command->identifier() == CheckIfNodeInBlackListCommand::identifier()){
        launchCheckIfNodeInBlackListTransaction(
            static_pointer_cast<CheckIfNodeInBlackListCommand>(
                command));

    } else if (command->identifier() == RemoveNodeFromBlackListCommand::identifier()){
        launchRemoveNodeFromBlackListTransaction(
            static_pointer_cast<RemoveNodeFromBlackListCommand>(
                command));

    } else if (command->identifier() == GetBlackListCommand::identifier()){
        launchGetBlackListTransaction(
            static_pointer_cast<GetBlackListCommand>(
                command));

    } else if (command->identifier() == PaymentTransactionByCommandUUIDCommand::identifier()){
        launchPaymentTransactionByCommandUUIDTransaction(
            static_pointer_cast<PaymentTransactionByCommandUUIDCommand>(
                command));

    } else {
        throw ValueError(
            "TransactionsManager::processCommand: "
                "Unexpected command identifier " + command->identifier());
    }
}

void TransactionsManager::processMessage(
    Message::Shared message)
{
    // ToDo: sort calls in the call probability order.
    // For example, max flows calculations would be called much often, then credit usage transactions.

    /*
     * Max flow
     */
    if (message->typeID() == Message::MessageType::MaxFlow_InitiateCalculation) {
        launchReceiveMaxFlowCalculationOnTargetTransaction(
            static_pointer_cast<InitiateMaxFlowCalculationMessage>(message));

    } else if (message->typeID() == Message::MessageType::MaxFlow_ResultMaxFlowCalculation) {
        try {
            mScheduler->tryAttachMessageToCollectTopologyTransaction(message);
        } catch (NotFoundError &) {
            launchReceiveResultMaxFlowCalculationTransaction(
                static_pointer_cast<ResultMaxFlowCalculationMessage>(message));
        }

    } else if (message->typeID() == Message::MessageType::MaxFlow_ResultMaxFlowCalculationFromGateway) {
        try {
            mScheduler->tryAttachMessageToCollectTopologyTransaction(message);
        } catch (NotFoundError &) {
            launchReceiveResultMaxFlowCalculationTransactionFromGateway(
                static_pointer_cast<ResultMaxFlowCalculationGatewayMessage>(message));
        }

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
    } else if (message->typeID() == Message::MessageType::Cycles_FiveNodesBoundary
               or message->typeID() == Message::MessageType::Cycles_SixNodesBoundary) {
        mScheduler->tryAttachMessageToCyclesFiveAndSixNodes(message);

    } else if (message->typeID() == Message::MessageType::Cycles_SixNodesMiddleware) {
        launchSixNodesCyclesResponseTransaction(
            static_pointer_cast<CyclesSixNodesInBetweenMessage>(message));

    } else if (message->typeID() == Message::MessageType::Cycles_FiveNodesMiddleware) {
        launchFiveNodesCyclesResponseTransaction(
            static_pointer_cast<CyclesFiveNodesInBetweenMessage>(message));

    } else if (message->typeID() == Message::MessageType::Cycles_ThreeNodesBalancesRequest){
        launchThreeNodesCyclesResponseTransaction(
            static_pointer_cast<CyclesThreeNodesBalancesRequestMessage>(message));

    } else if (message->typeID() == Message::MessageType::Cycles_FourNodesNegativeBalanceRequest){
        launchFourNodesCyclesResponseTransaction(
            static_pointer_cast<CyclesFourNodesNegativeBalanceRequestMessage>(message));

    } else if (message->typeID() == Message::MessageType::Cycles_FourNodesPositiveBalanceRequest){
        launchFourNodesCyclesResponseTransaction(
            static_pointer_cast<CyclesFourNodesPositiveBalanceRequestMessage>(message));

    /*
     * Trust lines
     */
    } else if (message->typeID() == Message::TrustLines_Initial) {
        launchAcceptTrustLineTransaction(
            static_pointer_cast<TrustLineInitialMessage>(message));

    } else if (message->typeID() == Message::TrustLines_PublicKeysSharingInit) {
        launchPublicKeysSharingTargetTransaction(
            static_pointer_cast<PublicKeysSharingInitMessage>(message));

    } else if (message->typeID() == Message::TrustLines_Audit) {
        launchAuditTargetTransaction(
            static_pointer_cast<AuditMessage>(message));

    } else if (message->typeID() == Message::TrustLines_ConflictResolver) {
        launchConflictResolveContractorTransaction(
            static_pointer_cast<ConflictResolverMessage>(message));

    /*
     * General
     */
    } else if (message->typeID() == Message::General_Pong) {
        launchPongReactionTransaction(
            static_pointer_cast<PongMessage>(message));

    /*
     * Gateway notification & RoutingTable
     */
    } else if (message->typeID() == Message::GatewayNotification) {
        launchGatewayNotificationReceiverTransaction(
            static_pointer_cast<GatewayNotificationMessage>(message));

    } else if (message->typeID() == Message::RoutingTableResponse) {
        try {
            mScheduler->tryAttachMessageToRoutingTableTransaction(message);
        } catch (NotFoundError &e) {
            launchRoutingTableUpdatingTransaction(
                static_pointer_cast<RoutingTableResponseMessage>(message));
        }

    /*
     * Attaching to existing transactions
     */
    } else {
        mScheduler->tryAttachMessageToTransaction(message);
    }
}

void TransactionsManager::launchInitTrustLineTransaction(
    InitTrustLineCommand::Shared command)
{
    try {
        mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent());
    } catch (NotFoundError &e) {
        info() << "launchInitTrustLineTransaction: init new equivalent "
               << command->equivalent();
        mEquivalentsSubsystemsRouter->initNewEquivalent(command->equivalent());
        mEquivalentsCyclesSubsystemsRouter->initNewEquivalent(command->equivalent());
    }

    auto transaction = make_shared<OpenTrustLineTransaction>(
        mNodeUUID,
        command,
        mContractorsManager,
        mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
        mStorageHandler,
        mEquivalentsSubsystemsRouter->iAmGateway(command->equivalent()),
        mSubsystemsController,
        mTrustLinesInfluenceController,
        mLog);
    subscribeForKeysSharingSignal(
        transaction->publicKeysSharingSignal);
    prepareAndSchedule(
        transaction,
        true,
        true,
        true);
}

void TransactionsManager::launchSetOutgoingTrustLineTransaction(
    SetOutgoingTrustLineCommand::Shared command)
{
    try {
        auto trustLinesManager = mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent());
        auto transaction = make_shared<SetOutgoingTrustLineTransaction>(
            mNodeUUID,
            command,
            mContractorsManager,
            trustLinesManager,
            mStorageHandler,
            mEquivalentsSubsystemsRouter->topologyCacheManager(command->equivalent()),
            mEquivalentsSubsystemsRouter->maxFlowCacheManager(command->equivalent()),
            mSubsystemsController,
            mKeysStore,
            mTrustLinesInfluenceController,
            mLog);
        subscribeForTrustLineActionSignal(
            transaction->trustLineActionSignal);
        prepareAndSchedule(
            transaction,
            true,
            true,
            true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for SetOutgoingTrustLineTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                command,
                mLog),
            false,
            false,
            false);
    }
}

void TransactionsManager::launchCloseIncomingTrustLineTransaction(
    CloseIncomingTrustLineCommand::Shared command)
{
    try {
        auto transaction = make_shared<CloseIncomingTrustLineTransaction>(
            mNodeUUID,
            command,
            mContractorsManager,
            mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
            mStorageHandler,
            mEquivalentsSubsystemsRouter->topologyTrustLineManager(command->equivalent()),
            mEquivalentsSubsystemsRouter->topologyCacheManager(command->equivalent()),
            mEquivalentsSubsystemsRouter->maxFlowCacheManager(command->equivalent()),
            mSubsystemsController,
            mKeysStore,
            mTrustLinesInfluenceController,
            mLog);
        subscribeForTrustLineActionSignal(
            transaction->trustLineActionSignal);
        prepareAndSchedule(
            transaction,
            true,
            true,
            true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for CloseIncomingTrustLineTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                command,
                mLog),
            false,
            false,
            false);
    }
}

void TransactionsManager::launchPublicKeysSharingSourceTransaction(
    ShareKeysCommand::Shared command)
{
    try {
        auto transaction = make_shared<PublicKeysSharingSourceTransaction>(
            mNodeUUID,
            command,
            mContractorsManager,
            mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
            mStorageHandler,
            mKeysStore,
            mTrustLinesInfluenceController,
            mLog);
        subscribeForAuditSignal(
            transaction->auditSignal);
        prepareAndSchedule(
            transaction,
            true,
            false,
            true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for PublicKeysSharingSourceTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                command,
                mLog),
            false,
            false,
            false);
    }
}

void TransactionsManager::launchAcceptTrustLineTransaction(
    TrustLineInitialMessage::Shared message)
{
    try {
        mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent());
    } catch (NotFoundError &e) {
        info() << "launchAcceptTrustLineTransaction: init new equivalent "
               << message->equivalent();
        mEquivalentsSubsystemsRouter->initNewEquivalent(message->equivalent());
        mEquivalentsCyclesSubsystemsRouter->initNewEquivalent(message->equivalent());
    }

    try {
        auto transaction = make_shared<AcceptTrustLineTransaction>(
            mNodeUUID,
            message,
            mContractorsManager,
            mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
            mStorageHandler,
            mEquivalentsSubsystemsRouter->iAmGateway(message->equivalent()),
            mSubsystemsController,
            mTrustLinesInfluenceController,
            mLog);
        prepareAndSchedule(
            transaction,
            false,
            false,
            true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for AcceptTrustLineTransaction "
                   "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

void TransactionsManager::launchPublicKeysSharingTargetTransaction(
    PublicKeysSharingInitMessage::Shared message)
{
    try {

        auto transaction = make_shared<PublicKeysSharingTargetTransaction>(
            mNodeUUID,
            message,
            mContractorsManager,
            mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
            mStorageHandler,
            mKeysStore,
            mTrustLinesInfluenceController,
            mLog);
        subscribeForKeysSharingSignal(
            transaction->publicKeysSharingSignal);
        prepareAndSchedule(
            transaction,
            false,
            false,
            true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for PublicKeysSharingTargetTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

void TransactionsManager::launchAuditTargetTransaction(
    AuditMessage::Shared message)
{
    try {
        auto transaction = make_shared<AuditTargetTransaction>(
            mNodeUUID,
            message,
            mContractorsManager,
            mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
            mStorageHandler,
            mKeysStore,
            mEquivalentsSubsystemsRouter->topologyTrustLineManager(message->equivalent()),
            mEquivalentsSubsystemsRouter->topologyCacheManager(message->equivalent()),
            mEquivalentsSubsystemsRouter->maxFlowCacheManager(message->equivalent()),
            mTrustLinesInfluenceController,
            mLog);
        subscribeForTrustLineActionSignal(
            transaction->trustLineActionSignal);
        prepareAndSchedule(
            transaction,
            false,
            false,
            true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for AuditTargetTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

void TransactionsManager::launchConflictResolveContractorTransaction(
    ConflictResolverMessage::Shared message)
{
    try {
        auto transaction = make_shared<ConflictResolverContractorTransaction>(
            mNodeUUID,
            message,
            mContractorsManager,
            mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
            mStorageHandler,
            mKeysStore,
            mTrustLinesInfluenceController,
            mLog);
        subscribeForProcessingConfirmationMessage(
            transaction->processConfirmationMessageSignal);
        prepareAndSchedule(
            transaction,
            false,
            true,
            true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for ConflictResolverContractorTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchInitiateMaxFlowCalculatingTransaction(
    InitiateMaxFlowCalculationCommand::Shared command)
{
    try {
        prepareAndSchedule(
            make_shared<InitiateMaxFlowCalculationTransaction>(
                mNodeUUID,
                command,
                mContractorsManager,
                mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
                mEquivalentsSubsystemsRouter->topologyTrustLineManager(command->equivalent()),
                mEquivalentsSubsystemsRouter->topologyCacheManager(command->equivalent()),
                mEquivalentsSubsystemsRouter->maxFlowCacheManager(command->equivalent()),
                mEquivalentsSubsystemsRouter->iAmGateway(command->equivalent()),
                mLog),
            true,
            true,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for InitiateMaxFlowCalculationTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                command,
                mLog),
            false,
            false,
            false);
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchMaxFlowCalculationFullyTransaction(
    InitiateMaxFlowCalculationFullyCommand::Shared command)
{
    try {
        prepareAndSchedule(
            make_shared<MaxFlowCalculationFullyTransaction>(
                mNodeUUID,
                command,
                mContractorsManager,
                mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
                mEquivalentsSubsystemsRouter->topologyTrustLineManager(command->equivalent()),
                mEquivalentsSubsystemsRouter->topologyCacheManager(command->equivalent()),
                mEquivalentsSubsystemsRouter->maxFlowCacheManager(command->equivalent()),
                mEquivalentsSubsystemsRouter->iAmGateway(command->equivalent()),
                mLog),
            true,
            true,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for MaxFlowCalculationFullyTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                command,
                mLog),
            false,
            false,
            false);
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchReceiveMaxFlowCalculationOnTargetTransaction(
    InitiateMaxFlowCalculationMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<ReceiveMaxFlowCalculationOnTargetTransaction>(
                mNodeUUID,
                message,
                mContractorsManager,
                mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
                mEquivalentsSubsystemsRouter->topologyCacheManager(message->equivalent()),
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for ReceiveMaxFlowCalculationOnTargetTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchReceiveResultMaxFlowCalculationTransaction(
    ResultMaxFlowCalculationMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<ReceiveResultMaxFlowCalculationTransaction>(
                mNodeUUID,
                message,
                mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
                mEquivalentsSubsystemsRouter->topologyTrustLineManager(message->equivalent()),
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for ReceiveResultMaxFlowCalculationTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchReceiveResultMaxFlowCalculationTransactionFromGateway(
    ResultMaxFlowCalculationGatewayMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<ReceiveResultMaxFlowCalculationTransaction>(
                mNodeUUID,
                message,
                mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
                mEquivalentsSubsystemsRouter->topologyTrustLineManager(message->equivalent()),
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for ReceiveResultMaxFlowCalculationTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchMaxFlowCalculationSourceFstLevelTransaction(
    MaxFlowCalculationSourceFstLevelMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<MaxFlowCalculationSourceFstLevelTransaction>(
                mNodeUUID,
                message,
                mContractorsManager,
                mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
                mLog,
                mEquivalentsSubsystemsRouter->iAmGateway(message->equivalent())),
            false,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for MaxFlowCalculationSourceFstLevelTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchMaxFlowCalculationTargetFstLevelTransaction(
    MaxFlowCalculationTargetFstLevelMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<MaxFlowCalculationTargetFstLevelTransaction>(
                mNodeUUID,
                message,
                mContractorsManager,
                mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
                mLog,
                mEquivalentsSubsystemsRouter->iAmGateway(message->equivalent())),
            false,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for MaxFlowCalculationTargetFstLevelTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchMaxFlowCalculationSourceSndLevelTransaction(
    MaxFlowCalculationSourceSndLevelMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<MaxFlowCalculationSourceSndLevelTransaction>(
                mNodeUUID,
                message,
                mContractorsManager,
                mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
                mEquivalentsSubsystemsRouter->topologyCacheManager(message->equivalent()),
                mLog,
                mEquivalentsSubsystemsRouter->iAmGateway(message->equivalent())),
            false,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for MaxFlowCalculationSourceSndLevelTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchMaxFlowCalculationTargetSndLevelTransaction(
    MaxFlowCalculationTargetSndLevelMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<MaxFlowCalculationTargetSndLevelTransaction>(
                mNodeUUID,
                message,
                mContractorsManager,
                mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
                mEquivalentsSubsystemsRouter->topologyCacheManager(message->equivalent()),
                mLog,
                mEquivalentsSubsystemsRouter->iAmGateway(message->equivalent())),
            false,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for MaxFlowCalculationTargetSndLevelTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

void TransactionsManager::launchCoordinatorPaymentTransaction(
    CreditUsageCommand::Shared command)
{
    try {
        auto transaction = make_shared<CoordinatorPaymentTransaction>(
            mNodeUUID,
            command,
            mEquivalentsSubsystemsRouter->iAmGateway(command->equivalent()),
            mContractorsManager,
            mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
            mStorageHandler,
            mEquivalentsSubsystemsRouter->topologyCacheManager(command->equivalent()),
            mEquivalentsSubsystemsRouter->maxFlowCacheManager(command->equivalent()),
            mResourcesManager,
            mEquivalentsSubsystemsRouter->pathsManager(command->equivalent()),
            mKeysStore,
            mLog,
            mSubsystemsController);
        subscribeForBuildCyclesThreeNodesTransaction(
            transaction->mBuildCycleThreeNodesSignal);
        subscribeForBuildCyclesFourNodesTransaction(
            transaction->mBuildCycleFourNodesSignal);
        subscribeForTrustLineActionSignal(
            transaction->trustLineActionSignal);
        prepareAndSchedule(transaction, true, false, true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for CoordinatorPaymentTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                command,
                mLog),
            false,
            false,
            false);
    }
}

void TransactionsManager::launchReceiverPaymentTransaction(
    ReceiverInitPaymentRequestMessage::Shared message)
{
    try {
        auto transaction = make_shared<ReceiverPaymentTransaction>(
            mNodeUUID,
            message,
            mEquivalentsSubsystemsRouter->iAmGateway(message->equivalent()),
            mContractorsManager,
            mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
            mStorageHandler,
            mEquivalentsSubsystemsRouter->topologyCacheManager(message->equivalent()),
            mEquivalentsSubsystemsRouter->maxFlowCacheManager(message->equivalent()),
            mKeysStore,
            mLog,
            mSubsystemsController);
        subscribeForBuildCyclesThreeNodesTransaction(
            transaction->mBuildCycleThreeNodesSignal);
        subscribeForBuildCyclesFourNodesTransaction(
            transaction->mBuildCycleFourNodesSignal);
        subscribeForTrustLineActionSignal(
            transaction->trustLineActionSignal);
        prepareAndSchedule(transaction, false, false, true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for ReceiverPaymentTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                message,
                mLog),
            false,
            false,
            true);
    }
}

void TransactionsManager::launchIntermediateNodePaymentTransaction(
    IntermediateNodeReservationRequestMessage::Shared message)
{
    try {
        auto transaction = make_shared<IntermediateNodePaymentTransaction>(
            mNodeUUID,
            message,
            mEquivalentsSubsystemsRouter->iAmGateway(message->equivalent()),
            mContractorsManager,
            mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
            mStorageHandler,
            mEquivalentsSubsystemsRouter->topologyCacheManager(message->equivalent()),
            mEquivalentsSubsystemsRouter->maxFlowCacheManager(message->equivalent()),
            mKeysStore,
            mLog,
            mSubsystemsController);
        subscribeForBuildCyclesThreeNodesTransaction(
            transaction->mBuildCycleThreeNodesSignal);
        subscribeForBuildCyclesFourNodesTransaction(
            transaction->mBuildCycleFourNodesSignal);
        subscribeForTrustLineActionSignal(
            transaction->trustLineActionSignal);
        prepareAndSchedule(transaction, false, false, true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for IntermediateNodePaymentTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                message,
                mLog),
            false,
            false,
            true);
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
                mContractorsManager,
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

void TransactionsManager::onCloseCycleTransaction(
    const SerializedEquivalent equivalent,
    Path::Shared cycle)
{
    try {
        auto transaction = make_shared<CycleCloserInitiatorTransaction>(
            mNodeUUID,
            cycle,
            equivalent,
            mContractorsManager,
            mEquivalentsSubsystemsRouter->trustLinesManager(equivalent),
            mEquivalentsCyclesSubsystemsRouter->cyclesManager(equivalent),
            mStorageHandler,
            mEquivalentsSubsystemsRouter->topologyCacheManager(equivalent),
            mEquivalentsSubsystemsRouter->maxFlowCacheManager(equivalent),
            mKeysStore,
            mLog,
            mSubsystemsController);
        subscribeForTrustLineActionSignal(
            transaction->trustLineActionSignal);
        prepareAndSchedule(
            transaction,
            true,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for CycleCloserInitiatorTransaction "
                "with equivalent " << equivalent << " Details are: " << e.what();
    }
}

void TransactionsManager::launchCycleCloserIntermediateNodeTransaction(
    IntermediateNodeCycleReservationRequestMessage::Shared message)
{
    try{
        auto transaction = make_shared<CycleCloserIntermediateNodeTransaction>(
            mNodeUUID,
            message,
            mContractorsManager,
            mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
            mEquivalentsCyclesSubsystemsRouter->cyclesManager(message->equivalent()),
            mStorageHandler,
            mEquivalentsSubsystemsRouter->topologyCacheManager(message->equivalent()),
            mEquivalentsSubsystemsRouter->maxFlowCacheManager(message->equivalent()),
            mKeysStore,
            mLog,
            mSubsystemsController);
        subscribeForTrustLineActionSignal(
            transaction->trustLineActionSignal);
        prepareAndSchedule(
            transaction,
            false,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for CycleCloserIntermediateNodeTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                message,
                mLog),
            false,
            false,
            true);
    }
}

void TransactionsManager::launchThreeNodesCyclesInitTransaction(
    ContractorID contractorID,
    const SerializedEquivalent equivalent)
{
    try {
        prepareAndSchedulePostponed(
            make_shared<CyclesThreeNodesInitTransaction>(
                mNodeUUID,
                contractorID,
                equivalent,
                mContractorsManager,
                mEquivalentsSubsystemsRouter->trustLinesManager(equivalent),
                mEquivalentsCyclesSubsystemsRouter->routingTableManager(equivalent),
                mEquivalentsCyclesSubsystemsRouter->cyclesManager(equivalent),
                mLog),
            50,
            true,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for CyclesThreeNodesInitTransaction "
                "with equivalent " << equivalent << " Details are: " << e.what();
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
                mContractorsManager,
                mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for CyclesThreeNodesReceiverTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

void TransactionsManager::launchSixNodesCyclesInitTransaction(
    const SerializedEquivalent equivalent)
{
    try {
        prepareAndSchedule(
            make_shared<CyclesSixNodesInitTransaction>(
                mNodeUUID,
                equivalent,
                mContractorsManager,
                mEquivalentsSubsystemsRouter->trustLinesManager(equivalent),
                mEquivalentsCyclesSubsystemsRouter->cyclesManager(equivalent),
                mLog),
            true,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for CyclesSixNodesInitTransaction "
                "with equivalent " << equivalent << " Details are: " << e.what();
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
                mContractorsManager,
                mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for CyclesSixNodesReceiverTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

void TransactionsManager::launchFiveNodesCyclesInitTransaction(
    const SerializedEquivalent equivalent)
{
    try {
        prepareAndSchedule(
            make_shared<CyclesFiveNodesInitTransaction>(
                mNodeUUID,
                equivalent,
                mContractorsManager,
                mEquivalentsSubsystemsRouter->trustLinesManager(equivalent),
                mEquivalentsCyclesSubsystemsRouter->cyclesManager(equivalent),
                mLog),
            true,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for CyclesFiveNodesInitTransaction "
                "with equivalent " << equivalent << " Details are: " << e.what();
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
                mContractorsManager,
                mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for CyclesFiveNodesReceiverTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

void TransactionsManager::launchFourNodesCyclesInitTransaction(
    ContractorID creditorID,
    const SerializedEquivalent equivalent)
{
    try {
        prepareAndSchedulePostponed(
            make_shared<CyclesFourNodesInitTransaction>(
                mNodeUUID,
                creditorID,
                equivalent,
                mContractorsManager,
                mEquivalentsSubsystemsRouter->trustLinesManager(equivalent),
                mEquivalentsCyclesSubsystemsRouter->routingTableManager(equivalent),
                mEquivalentsCyclesSubsystemsRouter->cyclesManager(equivalent),
                mLog),
            50,
            true,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for CyclesFourNodesInitTransaction "
                "with equivalent " << equivalent << " Details are: " << e.what();
    }
}

void TransactionsManager::launchFourNodesCyclesResponseTransaction(
    CyclesFourNodesNegativeBalanceRequestMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<CyclesFourNodesReceiverTransaction>(
                mNodeUUID,
                message,
                mContractorsManager,
                mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for CyclesFourNodesReceiverTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

void TransactionsManager::launchFourNodesCyclesResponseTransaction(
    CyclesFourNodesPositiveBalanceRequestMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<CyclesFourNodesReceiverTransaction>(
                mNodeUUID,
                message,
                mContractorsManager,
                mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for CyclesFourNodesReceiverTransaction (positive) "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchTotalBalancesTransaction(
    TotalBalancesCommand::Shared command)
{
    try {
        prepareAndSchedule(
            make_shared<TotalBalancesTransaction>(
                mNodeUUID,
                command,
                mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
                mLog),
            true,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for TotalBalancesTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                command,
                mLog),
            false,
            false,
            false);
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchHistoryPaymentsTransaction(
    HistoryPaymentsCommand::Shared command)
{
    try {
        mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for launchHistoryPaymentsTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                command,
                mLog),
            false,
            false,
            false);
        return;
    }
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

void TransactionsManager::launchAdditionalHistoryPaymentsTransaction(
    HistoryAdditionalPaymentsCommand::Shared command)
{
    try {
        mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for launchAdditionalHistoryPaymentsTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                command,
                mLog),
            false,
            false,
            false);
        return;
    }
    try {
        prepareAndSchedule(
            make_shared<HistoryAdditionalPaymentsTransaction>(
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
    HistoryTrustLinesCommand::Shared command)
{
    try {
        mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for launchHistoryTrustLinesTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                command,
                mLog),
            false,
            false,
            false);
        return;
    }
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
    HistoryWithContractorCommand::Shared command)
{
    try {
        mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for launchHistoryWithContractorTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                command,
                mLog),
            false,
            false,
            false);
        return;
    }
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
                mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
                mLog),
            true,
            false,
            false);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for GetFirstLevelContractorsTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                command,
                mLog),
            false,
            false,
            false);
    }
}


void TransactionsManager::launchGetTrustLinesTransaction(
    GetTrustLinesCommand::Shared command)
{
    try {
        prepareAndSchedule(
            make_shared<GetFirstLevelContractorsBalancesTransaction>(
                mNodeUUID,
                command,
                mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
                mLog),
            true,
            false,
            false);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for GetFirstLevelContractorsBalancesTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                command,
                mLog),
            false,
            false,
            false);
    }

}

void TransactionsManager::launchGetTrustLineTransaction(
    GetTrustLineCommand::Shared command)
{
    try {
        prepareAndSchedule(
            make_shared<GetFirstLevelContractorBalanceTransaction>(
                mNodeUUID,
                command,
                mContractorsManager,
                mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
                mLog),
            true,
            false,
            false);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for GetFirstLevelContractorBalanceTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                command,
                mLog),
            false,
            false,
            false);
    }
}

void TransactionsManager::launchGetEquivalentListTransaction(
    EquivalentListCommand::Shared command)
{
    prepareAndSchedule(
        make_shared<GetEquivalentListTransaction>(
            mNodeUUID,
            command,
            mEquivalentsSubsystemsRouter,
            mLog),
        true,
        false,
        false);
}

void TransactionsManager::launchAddNodeToBlackListTransaction(
    AddNodeToBlackListCommand::Shared command)
{
    try {
        prepareAndSchedule(
            make_shared<AddNodeToBlackListTransaction>(
                mNodeUUID,
                command,
                mContractorsManager,
                mStorageHandler,
                mEquivalentsSubsystemsRouter,
                mSubsystemsController,
                mTrustLinesInfluenceController,
                mKeysStore,
                mLog),
            true,
            true,
            false);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }
}

void TransactionsManager::launchCheckIfNodeInBlackListTransaction(
    CheckIfNodeInBlackListCommand::Shared command)
{
    try {
        prepareAndSchedule(
            make_shared<CheckIfNodeInBlackListTransaction>(
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

void TransactionsManager::launchRemoveNodeFromBlackListTransaction(
    RemoveNodeFromBlackListCommand::Shared command)
{
    try {
        prepareAndSchedule(
            make_shared<RemoveNodeFromBlackListTransaction>(
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

void TransactionsManager::launchGetBlackListTransaction(
    GetBlackListCommand::Shared command)
{
    try {
        prepareAndSchedule(
            make_shared<GetBlackListTransaction>(
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

void TransactionsManager::launchPaymentTransactionByCommandUUIDTransaction(
    PaymentTransactionByCommandUUIDCommand::Shared command)
{
    try {
        prepareAndSchedule(
            make_shared<PaymentTransactionByCommandUUIDTransaction>(
                mNodeUUID,
                command,
                mScheduler->paymentTransactionByCommandUUID(
                    command->paymentTransactionCommandUUID()),
                mLog),
            false,
            false,
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
                mContractorsManager,
                mEquivalentsSubsystemsRouter,
                mEquivalentsCyclesSubsystemsRouter.get(),
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
                mContractorsManager,
                mEquivalentsSubsystemsRouter,
                mStorageHandler,
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    }
}

void TransactionsManager::launchRoutingTableUpdatingTransaction(
    RoutingTableResponseMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<RoutingTableUpdatingTransaction>(
                mNodeUUID,
                message,
                mEquivalentsSubsystemsRouter,
                mEquivalentsCyclesSubsystemsRouter.get(),
                mLog),
            false,
            false,
            false);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    }
}

void TransactionsManager::launchFindPathByMaxFlowTransaction(
    const TransactionUUID &requestedTransactionUUID,
    BaseAddress::Shared destinationNodeAddress,
    const SerializedEquivalent equivalent)
{
    try {
        prepareAndSchedule(
            make_shared<FindPathByMaxFlowTransaction>(
                mNodeUUID,
                destinationNodeAddress,
                requestedTransactionUUID,
                equivalent,
                mContractorsManager,
                mEquivalentsSubsystemsRouter->pathsManager(equivalent),
                mResourcesManager,
                mEquivalentsSubsystemsRouter->trustLinesManager(equivalent),
                mEquivalentsSubsystemsRouter->topologyTrustLineManager(equivalent),
                mEquivalentsSubsystemsRouter->topologyCacheManager(equivalent),
                mEquivalentsSubsystemsRouter->maxFlowCacheManager(equivalent),
                mEquivalentsSubsystemsRouter->iAmGateway(equivalent),
                mLog),
            true,
            true,
            false);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for FindPathByMaxFlowTransaction "
                "with equivalent " << equivalent << " Details are: " << e.what();
    }
}

void TransactionsManager::launchPongReactionTransaction(
    PongMessage::Shared message)
{
    auto transaction = make_shared<PongReactionTransaction>(
        mNodeUUID,
        message,
        mEquivalentsSubsystemsRouter,
        mStorageHandler,
        mLog);
    subscribeForProcessingPongMessage(
        transaction->processPongMessageSignal);
    transaction->mResumeTransactionSignal.connect(
        boost::bind(
            &TransactionsManager::onResumeTransactionSlot,
            this,
            _1,
            _2,
            _3));
    prepareAndSchedule(
        transaction,
        true,
        false,
        false);
}

void TransactionsManager::attachResourceToTransaction(
    BaseResource::Shared resource)
{
    mScheduler->tryAttachResourceToTransaction(
        resource);
}

void TransactionsManager::subscribeForSubsidiaryTransactions(
    BaseTransaction::LaunchSubsidiaryTransactionSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onSubsidiaryTransactionReady,
            this,
            _1));
}

void TransactionsManager::subscribeForOutgoingMessages(
    BaseTransaction::SendMessageSignal &signal)
{
    // ToDo: connect signals of transaction and core directly (signal -> signal)
    // Boost allows this type of connectivity.
    signal.connect(
        boost::bind(
            &TransactionsManager::onTransactionOutgoingMessageReady,
            this,
            _1,
            _2));
}

void TransactionsManager::subscribeForOutgoingNewMessages(
    BaseTransaction::SendMessageNewSignal &signal)
{
    // ToDo: connect signals of transaction and core directly (signal -> signal)
    // Boost allows this type of connectivity.
    signal.connect(
        boost::bind(
            &TransactionsManager::onTransactionOutgoingNewMessageReady,
            this,
            _1,
            _2));
}

void TransactionsManager::subscribeForOutgoingMessagesToAddress(
    BaseTransaction::SendMessageToAddressSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onTransactionOutgoingMessageToAddressReady,
            this,
            _1,
            _2));
}

void TransactionsManager::subscribeForOutgoingMessagesWithCaching(
    BaseTransaction::SendMessageWithCachingSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onTransactionOutgoingMessageWithCachingReady,
            this,
            _1,
            _2,
            _3,
            _4));
}

void TransactionsManager::subscribeForSerializeTransaction(
    TransactionsScheduler::SerializeTransactionSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onSerializeTransaction,
            this,
            _1));
}

void TransactionsManager::subscribeForCommandResult(
    TransactionsScheduler::CommandResultSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onCommandResultReady,
            this,
            _1));
}

void TransactionsManager::subscribeForBuildCyclesThreeNodesTransaction(
    BasePaymentTransaction::BuildCycleThreeNodesSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onBuildCycleThreeNodesTransaction,
            this,
            _1,
            _2));
}

void TransactionsManager::subscribeForBuildCyclesFourNodesTransaction(
    BasePaymentTransaction::BuildCycleFourNodesSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onBuildCycleFourNodesTransaction,
            this,
            _1,
            _2));
}

void TransactionsManager::subscribeForBuildCyclesFiveNodesTransaction(
    EquivalentsCyclesSubsystemsRouter::BuildFiveNodesCyclesSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onBuildCycleFiveNodesTransaction,
            this,
            _1));
}

void TransactionsManager::subscribeForBuildCyclesSixNodesTransaction(
    EquivalentsCyclesSubsystemsRouter::BuildSixNodesCyclesSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onBuildCycleSixNodesTransaction,
            this,
            _1));
}

void TransactionsManager::subscribeForCloseCycleTransaction(
    EquivalentsCyclesSubsystemsRouter::CloseCycleSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onCloseCycleTransaction,
            this,
            _1,
            _2));
}

void TransactionsManager::subscribeForTryCloseNextCycleSignal(
    TransactionsScheduler::CycleCloserTransactionWasFinishedSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onTryCloseNextCycleSlot,
            this,
            _1));
}

void TransactionsManager::subscribeForProcessingConfirmationMessage(
    BaseTransaction::ProcessConfirmationMessageSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onProcessConfirmationMessageSlot,
            this,
            _1));
}

void TransactionsManager::subscribeForProcessingPongMessage(
    BaseTransaction::ProcessPongMessageSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onProcessPongMessageSlot,
            this,
            _1));
}

void TransactionsManager::subscribeForGatewayNotificationSignal(
    EquivalentsSubsystemsRouter::GatewayNotificationSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onGatewayNotificationSlot,
            this));
}

void TransactionsManager::subscribeForTrustLineActionSignal(
    BasePaymentTransaction::TrustLineActionSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onTrustLineActionSlot,
            this,
            _1,
            _2,
            _3));
}

void TransactionsManager::subscribeForKeysSharingSignal(
    BaseTransaction::PublicKeysSharingSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onPublicKeysSharingSlot,
            this,
            _1,
            _2));
}

void TransactionsManager::subscribeForAuditSignal(
    BaseTransaction::AuditSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onAuditSlot,
            this,
            _1,
            _2));
}

void TransactionsManager::onTransactionOutgoingMessageReady(
    Message::Shared message,
    const NodeUUID &contractorUUID)
{
    transactionOutgoingMessageReadySignal(
        message,
        contractorUUID);
}

void TransactionsManager::onTransactionOutgoingNewMessageReady(
    Message::Shared message,
    const ContractorID contractorID)
{
    transactionOutgoingMessageReadyNewSignal(
        message,
        contractorID);
}

void TransactionsManager::onTransactionOutgoingMessageToAddressReady(
    Message::Shared message,
    BaseAddress::Shared address)
{
    transactionOutgoingMessageToAddressReadySignal(
        message,
        address);
}

void TransactionsManager::onTransactionOutgoingMessageWithCachingReady(
    TransactionMessage::Shared message,
    ContractorID contractorID,
    Message::MessageType incomingMessageTypeFilter,
    uint32_t cacheLivingTime)
{
    transactionOutgoingMessageWithCachingReadySignal(
        message,
        contractorID,
        incomingMessageTypeFilter,
        cacheLivingTime);
}

/*!
 * Writes received result to the outgoing results fifo.
 *
 * Throws RuntimeError - in case if result can't be processed.
 */
void TransactionsManager::onCommandResultReady(
    CommandResult::SharedConst result)
{
    try {
        auto message = result->serialize();

        info() << "Result for command " + result->identifier();

        if (result->identifier() == HistoryPaymentsCommand::identifier() or
                result->identifier() == HistoryTrustLinesCommand::identifier() or
                result->identifier() == HistoryWithContractorCommand::identifier() or
                result->identifier() == HistoryAdditionalPaymentsCommand::identifier() or
                result->identifier() == GetTrustLinesCommand::identifier()) {
            auto shortMessage = result->serializeShort();
            info() << "CommandResultReady: " << shortMessage;
        } else {
            info() << "CommandResultReady: " << message;
        }

        mResultsInterface->writeResult(
            message.c_str(),
            message.size());

    } catch (...) {
        throw RuntimeError(
            "TransactionsManager::onCommandResultReady: "
                "Error occurred when command result has accepted");
    }
}

void TransactionsManager::onSubsidiaryTransactionReady(
    BaseTransaction::Shared transaction)
{
    subscribeForSubsidiaryTransactions(
        transaction->runSubsidiaryTransactionSignal);

    subscribeForOutgoingMessages(
        transaction->outgoingMessageIsReadySignal);
    subscribeForOutgoingNewMessages(
        transaction->outgoingMessageIsReadyNewSignal);
    subscribeForOutgoingMessagesToAddress(
        transaction->outgoingMessageToAddressReadySignal);
    subscribeForOutgoingMessagesWithCaching(
        transaction->sendMessageWithCachingSignal);

    subscribeForProcessingConfirmationMessage(
        transaction->processConfirmationMessageSignal);

    subscribeForTrustLineActionSignal(
        transaction->trustLineActionSignal);

    mScheduler->postponeTransaction(
        transaction,
        50);
}

void TransactionsManager::onBuildCycleThreeNodesTransaction(
    set<ContractorID> &contractorsIDs,
    const SerializedEquivalent equivalent)
{
    for (const auto &contractorID : contractorsIDs) {
        launchThreeNodesCyclesInitTransaction(
            contractorID,
            equivalent);
    }
}

void TransactionsManager::onBuildCycleFourNodesTransaction(
    set<ContractorID> &contractorsIDs,
    const SerializedEquivalent equivalent)
{
    for (const auto &contractorID : contractorsIDs) {
        launchFourNodesCyclesInitTransaction(
            contractorID,
            equivalent);
    }
}

void TransactionsManager::onBuildCycleFiveNodesTransaction(
    const SerializedEquivalent equivalent)
{
    launchFiveNodesCyclesInitTransaction(
        equivalent);
}

void TransactionsManager::onBuildCycleSixNodesTransaction(
    const SerializedEquivalent equivalent)
{
    launchSixNodesCyclesInitTransaction(
        equivalent);
}

void TransactionsManager::onTryCloseNextCycleSlot(
    const SerializedEquivalent equivalent)
{
    try {
        mEquivalentsCyclesSubsystemsRouter->cyclesManager(equivalent)->closeOneCycle(true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for Closing next cycle "
                "with equivalent " << equivalent << " Details are: " << e.what();
    }
}

void TransactionsManager::onProcessConfirmationMessageSlot(
    ConfirmationMessage::Shared confirmationMessage)
{
    ProcessConfirmationMessageSignal(
        confirmationMessage);
}

void TransactionsManager::onProcessPongMessageSlot(
    ContractorID contractorID)
{
    ProcessPongMessageSignal(
        contractorID);
}

void TransactionsManager::onGatewayNotificationSlot()
{
    launchGatewayNotificationSenderTransaction();
}

void TransactionsManager::onTrustLineActionSlot(
    ContractorID contractorID,
    const SerializedEquivalent equivalent,
    bool isActionInitiator)
{
    try {
        auto trustLinesManager = mEquivalentsSubsystemsRouter->trustLinesManager(equivalent);
        auto transaction = make_shared<CheckTrustLineTransaction>(
            mNodeUUID,
            equivalent,
            contractorID,
            isActionInitiator,
            trustLinesManager,
            mLog);

        subscribeForKeysSharingSignal(
            transaction->publicKeysSharingSignal);

        subscribeForAuditSignal(
            transaction->auditSignal);

        // wait for finish of contractor payment TA
        mScheduler->postponeTransaction(
            transaction,
            500);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for onTrustLineActionSlot "
                "with equivalent " << equivalent << " Details are: " << e.what();
    }
}

void TransactionsManager::onPublicKeysSharingSlot(
    ContractorID contractorID,
    const SerializedEquivalent equivalent)
{
    try {
        auto trustLinesManager = mEquivalentsSubsystemsRouter->trustLinesManager(equivalent);
        auto transaction = make_shared<PublicKeysSharingSourceTransaction>(
            mNodeUUID,
            contractorID,
            equivalent,
            mContractorsManager,
            trustLinesManager,
            mStorageHandler,
            mKeysStore,
            mTrustLinesInfluenceController,
            mLog);

        subscribeForOutgoingMessages(
            transaction->outgoingMessageIsReadySignal);
        subscribeForOutgoingNewMessages(
            transaction->outgoingMessageIsReadyNewSignal);
        subscribeForOutgoingMessagesWithCaching(
            transaction->sendMessageWithCachingSignal);

        subscribeForAuditSignal(
            transaction->auditSignal);

        mScheduler->postponeTransaction(
            transaction,
            5);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for onPublicKeysSharingSlot "
                "with equivalent " << equivalent << " Details are: " << e.what();
    }
}

void TransactionsManager::onAuditSlot(
    ContractorID contractorID,
    const SerializedEquivalent equivalent)
{
    try {
        auto trustLinesManager = mEquivalentsSubsystemsRouter->trustLinesManager(equivalent);
        auto transaction = make_shared<AuditSourceTransaction>(
            mNodeUUID,
            contractorID,
            equivalent,
            mContractorsManager,
            trustLinesManager,
            mStorageHandler,
            mKeysStore,
            mTrustLinesInfluenceController,
            mLog);

        subscribeForTrustLineActionSignal(
            transaction->trustLineActionSignal);
        subscribeForProcessingPongMessage(
            transaction->processPongMessageSignal);

        prepareAndSchedule(
            transaction,
            true,
            false,
            true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for onAuditSlot "
                "with equivalent " << equivalent << " Details are: " << e.what();
    }
}

void TransactionsManager::onResumeTransactionSlot(
    ContractorID contractorID,
    const SerializedEquivalent equivalent,
    const BaseTransaction::TransactionType transactionType)
{
    switch (transactionType) {
        case BaseTransaction::OpenTrustLineTransaction: {
            auto transaction = make_shared<OpenTrustLineTransaction>(
                mNodeUUID,
                equivalent,
                contractorID,
                mContractorsManager,
                mEquivalentsSubsystemsRouter->trustLinesManager(equivalent),
                mStorageHandler,
                mEquivalentsSubsystemsRouter->iAmGateway(equivalent),
                mSubsystemsController,
                mTrustLinesInfluenceController,
                mLog);
            subscribeForKeysSharingSignal(
                transaction->publicKeysSharingSignal);
            subscribeForProcessingPongMessage(
                transaction->processPongMessageSignal);
            prepareAndSchedule(
                transaction,
                true,
                false,
                true);
            break;
        }
        case BaseTransaction::AuditSourceTransactionType: {
            auto transaction = make_shared<AuditSourceTransaction>(
                mNodeUUID,
                equivalent,
                mContractorsManager,
                contractorID,
                mEquivalentsSubsystemsRouter->trustLinesManager(equivalent),
                mStorageHandler,
                mKeysStore,
                mTrustLinesInfluenceController,
                mLog);
            subscribeForTrustLineActionSignal(
                transaction->trustLineActionSignal);
            subscribeForProcessingPongMessage(
                transaction->processPongMessageSignal);
            prepareAndSchedule(
                transaction,
                true,
                false,
                true);
            break;
        }
        default: {
            warning() << "onResumeTransactionSlot: invalid transaction type " << transactionType;
        }
    }
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
    if (outgoingMessagesSubscribe) {
        subscribeForOutgoingMessages(
            transaction->outgoingMessageIsReadySignal);
        subscribeForOutgoingNewMessages(
            transaction->outgoingMessageIsReadyNewSignal);
        subscribeForOutgoingMessagesToAddress(
            transaction->outgoingMessageToAddressReadySignal);
        subscribeForOutgoingMessagesWithCaching(
            transaction->sendMessageWithCachingSignal);
    }
    if (subsidiaryTransactionSubscribe) {
        subscribeForSubsidiaryTransactions(
            transaction->runSubsidiaryTransactionSignal);
    }

    while (true) {
        try {

            mScheduler->scheduleTransaction(
                transaction);
            return;

        } catch(bad_alloc &) {
            throw bad_alloc();

        } catch(ConflictError &e) {
            if (regenerateUUID) {
                warning() << "prepareAndSchedule:" << "TransactionUUID: " << transaction->currentTransactionUUID()
                          << " New TransactionType:" << transaction->transactionType()
                          << " Recreate.";
                transaction->recreateTransactionUUID();
            } else {
                warning() << "prepareAndSchedule:" << "TransactionUUID: " << transaction->currentTransactionUUID()
                          << " New TransactionType:" << transaction->transactionType()
                          << " Exit.";
                return;
            }
        }
    }
}

void TransactionsManager::prepareAndSchedulePostponed(
    BaseTransaction::Shared transaction,
    uint32_t millisecondsDelay,
    bool regenerateUUID,
    bool subsidiaryTransactionSubscribe,
    bool outgoingMessagesSubscribe)
{
    if (outgoingMessagesSubscribe) {
        subscribeForOutgoingMessages(
            transaction->outgoingMessageIsReadySignal);
        subscribeForOutgoingNewMessages(
            transaction->outgoingMessageIsReadyNewSignal);
        subscribeForOutgoingMessagesToAddress(
            transaction->outgoingMessageToAddressReadySignal);
        subscribeForOutgoingMessagesWithCaching(
            transaction->sendMessageWithCachingSignal);
    }
    if (subsidiaryTransactionSubscribe) {
        subscribeForSubsidiaryTransactions(
            transaction->runSubsidiaryTransactionSignal);
    }

    while (true) {
        try {

            mScheduler->postponeTransaction(
                transaction,
                millisecondsDelay);
            return;

        } catch(bad_alloc &) {
            throw bad_alloc();

        } catch(ConflictError &e) {
            if (regenerateUUID) {
                warning() << "prepareAndSchedulePostponed:" << "TransactionUUID: " << transaction->currentTransactionUUID()
                          << " New TransactionType:" << transaction->transactionType()
                          << " Recreate.";
                transaction->recreateTransactionUUID();
            } else {
                warning() << "prepareAndSchedulePostponed:" << "TransactionUUID: " << transaction->currentTransactionUUID()
                          << " New TransactionType:" << transaction->transactionType()
                          << " Exit.";
                return;
            }
        }
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
                    "Unexpected transaction type identifier " + to_string(kTransactionTypeId));
        }
    }
}

string TransactionsManager::logHeader()
    noexcept
{
    return "[TransactionsManager]";
}

LoggerStream TransactionsManager::error() const
    noexcept
{
    return mLog.error(logHeader());
}

LoggerStream TransactionsManager::warning() const
    noexcept
{
    return mLog.warning(logHeader());
}

LoggerStream TransactionsManager::info() const
    noexcept
{
    return mLog.info(logHeader());
}