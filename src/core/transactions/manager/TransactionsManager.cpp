#include "TransactionsManager.h"

/*!
 *
 * Throws RuntimeError in case if some internal components can't be initialised.
 */
TransactionsManager::TransactionsManager(
    as::io_service &IOService,
    ContractorsManager *contractorsManager,
    EquivalentsSubsystemsRouter *equivalentsSubsystemsRouter,
    ResourcesManager *resourcesManager,
    ResultsInterface *resultsInterface,
    StorageHandler *storageHandler,
    Keystore *keystore,
    FeaturesManager *featuresManager,
    EventsInterface *eventsInterface,
    TailManager *tailManager,
    Logger &logger,
    SubsystemsController *subsystemsController,
    TrustLinesInfluenceController *trustLinesInfluenceController) :

    mIOService(IOService),
    mContractorsManager(contractorsManager),
    mEquivalentsSubsystemsRouter(equivalentsSubsystemsRouter),
    mResourcesManager(resourcesManager),
    mResultsInterface(resultsInterface),
    mStorageHandler(storageHandler),
    mKeysStore(keystore),
    mFeaturesManager(featuresManager),
    mEventsInterface(eventsInterface),
    mTailManager(tailManager),
    mLog(logger),
    mSubsystemsController(subsystemsController),
    mTrustLinesInfluenceController(trustLinesInfluenceController),
    isPaymentTransactionsAllowedDueToObserving(false),

    mScheduler(
        new TransactionsScheduler(
            mIOService,
            mTrustLinesInfluenceController,
            mLog)),

    mEquivalentsCyclesSubsystemsRouter(
        new EquivalentsCyclesSubsystemsRouter(
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
            case BaseTransaction::IntermediateNodePaymentTransaction:
            case BaseTransaction::ReceiverPaymentTransaction:
            case BaseTransaction::Payments_CycleCloserIntermediateNodeTransaction: {
                auto paymentTransaction = deserializePaymentTransaction(
                    kTABuffer,
                    transactionTypeId,
                    *equivalent);
                if (paymentTransaction == nullptr) {
                    continue;
                }
                prepareAndSchedule(
                    paymentTransaction,
                    false,
                    false,
                    true);
                break;
            }
            case BaseTransaction::ConflictResolverInitiatorTransactionType: {
                try {
                    auto transaction = make_shared<ConflictResolverInitiatorTransaction>(
                        kTABuffer,
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

BasePaymentTransaction::Shared TransactionsManager::deserializePaymentTransaction(
    BytesShared buffer,
    BaseTransaction::SerializedTransactionType transactionType,
    SerializedEquivalent equivalent)
{
    switch (transactionType) {
        case BaseTransaction::IntermediateNodePaymentTransaction: {
            try {
                auto transaction = make_shared<IntermediateNodePaymentTransaction>(
                    buffer,
                    mEquivalentsSubsystemsRouter->iAmGateway(equivalent),
                    mContractorsManager,
                    mEquivalentsSubsystemsRouter->trustLinesManager(equivalent),
                    mStorageHandler,
                    mEquivalentsSubsystemsRouter->topologyCacheManager(equivalent),
                    mEquivalentsSubsystemsRouter->maxFlowCacheManager(equivalent),
                    mResourcesManager,
                    mKeysStore,
                    mLog,
                    mSubsystemsController);
                subscribeForBuildCyclesThreeNodesTransaction(
                    transaction->mBuildCycleThreeNodesSignal);
                subscribeForBuildCyclesFourNodesTransaction(
                    transaction->mBuildCycleFourNodesSignal);
                subscribeForTrustLineActionSignal(
                    transaction->trustLineActionSignal);
                subscribeForObserving(
                    transaction);
                return transaction;
            } catch (NotFoundError &e) {
                error() << "There are no subsystems for serialized IntermediateNodePaymentTransaction "
                        "with equivalent " << equivalent << " Details are: " << e.what();
                return nullptr;
            }
        }
        case BaseTransaction::ReceiverPaymentTransaction: {
            try {
                auto transaction = make_shared<ReceiverPaymentTransaction>(
                    buffer,
                    mEquivalentsSubsystemsRouter->iAmGateway(equivalent),
                    mContractorsManager,
                    mEquivalentsSubsystemsRouter->trustLinesManager(equivalent),
                    mStorageHandler,
                    mEquivalentsSubsystemsRouter->topologyCacheManager(equivalent),
                    mEquivalentsSubsystemsRouter->maxFlowCacheManager(equivalent),
                    mResourcesManager,
                    mKeysStore,
                    mEventsInterface,
                    mLog,
                    mSubsystemsController);
                subscribeForBuildCyclesThreeNodesTransaction(
                    transaction->mBuildCycleThreeNodesSignal);
                subscribeForBuildCyclesFourNodesTransaction(
                    transaction->mBuildCycleFourNodesSignal);
                subscribeForTrustLineActionSignal(
                    transaction->trustLineActionSignal);
                subscribeForObserving(
                    transaction);
                return transaction;
            } catch (NotFoundError &e) {
                error() << "There are no subsystems for serialized ReceiverPaymentTransaction "
                        "with equivalent " << equivalent << " Details are: " << e.what();
                return nullptr;
            }
        }
        case BaseTransaction::Payments_CycleCloserIntermediateNodeTransaction: {
            try {
                auto transaction = make_shared<CycleCloserIntermediateNodeTransaction>(
                    buffer,
                    mContractorsManager,
                    mEquivalentsSubsystemsRouter->trustLinesManager(equivalent),
                    mEquivalentsCyclesSubsystemsRouter->cyclesManager(equivalent),
                    mStorageHandler,
                    mEquivalentsSubsystemsRouter->topologyCacheManager(equivalent),
                    mEquivalentsSubsystemsRouter->maxFlowCacheManager(equivalent),
                    mResourcesManager,
                    mKeysStore,
                    mLog,
                    mSubsystemsController);
                subscribeForTrustLineActionSignal(
                    transaction->trustLineActionSignal);
                subscribeForObserving(
                    transaction);
                return transaction;
            } catch (NotFoundError &e) {
                error() << "There are no subsystems for serialized CycleCloserIntermediateNodeTransaction "
                        "with equivalent " << equivalent << " Details are: " << e.what();
                return nullptr;
            }
        }
        default: {
            throw RuntimeError(
                    "TrustLinesManager::deserializePaymentTransaction. "
                        "Unexpected transaction type identifier " + to_string(transactionType));
        }
    }
}

void TransactionsManager::processCommand(
    bool success,
    BaseUserCommand::Shared command)
{
    if(!success and command) {
        return onCommandResultReady(
            ((ErrorUserCommand *)command.get())->responseError());
    }

    // ToDo: sort calls in the call probability order.
    // For example, max flows calculations would be called much often, then credit usage transactions.
    // So, why we are checking trust lines commands first, and max flow is only in the middle of the check sequence?
    if (command->identifier() == InitChannelCommand::identifier()) {
        launchInitChannelTransaction(
            static_pointer_cast<InitChannelCommand>(
                command));

    } else if (command->identifier() == SetChannelContractorAddressesCommand::identifier()) {
        launchSetChannelContractorAddressesTransaction(
            static_pointer_cast<SetChannelContractorAddressesCommand>(
                command));

    } else if (command->identifier() == SetChannelContractorCryptoKeyCommand::identifier()) {
        launchSetChannelContractorCryptoKeyTransaction(
            static_pointer_cast<SetChannelContractorCryptoKeyCommand>(
                command));

    } else if (command->identifier() == RegenerateChannelCryptoKeyCommand::identifier()) {
        launchRegenerateChannelCryptoKeyTransaction(
            static_pointer_cast<RegenerateChannelCryptoKeyCommand>(
                command));

    } else if (command->identifier() == InitTrustLineCommand::identifier()) {
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

    } else if (command->identifier() == RemoveTrustLineCommand::identifier()) {
        launchRemoveTrustLineTransaction(
            static_pointer_cast<RemoveTrustLineCommand>(
                command));

    } else if (command->identifier() == ResetTrustLineCommand::identifier()) {
        launchResetTrustLineSourceTransaction(
            static_pointer_cast<ResetTrustLineCommand>(
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

    } else if (command->identifier() == GetTrustLineByAddressCommand::identifier()){
        launchGetTrustLineByAddressTransaction(
            static_pointer_cast<GetTrustLineByAddressCommand>(
                command));

    } else if (command->identifier() == GetTrustLineByIDCommand::identifier()){
        launchGetTrustLineByIDTransaction(
            static_pointer_cast<GetTrustLineByIDCommand>(
                command));

    } else if (command->identifier() == EquivalentListCommand::identifier()){
        launchGetEquivalentListTransaction(
            static_pointer_cast<EquivalentListCommand>(
                command));

    } else if (command->identifier() == ContractorListCommand::identifier()){
        launchGetContractorListTransaction(
            static_pointer_cast<ContractorListCommand>(
                command));

    } else if (command->identifier() == GetChannelInfoCommand::identifier()){
        launchGetChannelInfoTransaction(
            static_pointer_cast<GetChannelInfoCommand>(
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

    } else if (message->typeID() == Message::TrustLines_Reset) {
        launchResetTrustLineDestinationTransaction(
            static_pointer_cast<TrustLineResetMessage>(message));

    /*
     * Channels
     */
    } else if (message->typeID() == Message::Channel_Init) {
        launchConfirmChannelTransaction(
            static_pointer_cast<InitChannelMessage>(message));

    } else if (message->typeID() == Message::Channel_UpdateAddresses) {
        launchUpdateChannelAddressesTargetTransaction(
            static_pointer_cast<UpdateChannelAddressesMessage>(message));

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

    /*
     * Attaching to existing transactions
     */
    } else {
        mScheduler->tryAttachMessageToTransaction(message);
    }
}

void TransactionsManager::launchInitChannelTransaction(
    InitChannelCommand::Shared command)
{
    auto transaction = make_shared<InitChannelTransaction>(
        command,
        mContractorsManager,
        mStorageHandler,
        mLog);
    prepareAndSchedule(
        transaction,
        true,
        false,
        true);
}

void TransactionsManager::launchConfirmChannelTransaction(
    InitChannelMessage::Shared message)
{
    auto transaction = make_shared<ConfirmChannelTransaction>(
        message,
        mContractorsManager,
        mStorageHandler,
        mLog);
    prepareAndSchedule(
        transaction,
        false,
        false,
        true);
}

void TransactionsManager::launchGetContractorListTransaction(
    ContractorListCommand::Shared command)
{
    prepareAndSchedule(
        make_shared<GetContractorListTransaction>(
            command,
            mContractorsManager,
            mLog),
        true,
        false,
        false);
}

void TransactionsManager::launchGetChannelInfoTransaction(
    GetChannelInfoCommand::Shared command)
{
    prepareAndSchedule(
        make_shared<GetChannelInfoTransaction>(
            command,
            mContractorsManager,
            mLog),
        true,
        false,
        false);
}

void TransactionsManager::launchUpdateChannelAddressesTargetTransaction(
    UpdateChannelAddressesMessage::Shared message)
{
    prepareAndSchedule(
        make_shared<UpdateChannelAddressesTargetTransaction>(
            message,
            mContractorsManager,
            mStorageHandler,
            mLog),
        false,
        false,
        true);
}

void TransactionsManager::launchSetChannelContractorAddressesTransaction(
    SetChannelContractorAddressesCommand::Shared command)
{
    prepareAndSchedule(
        make_shared<SetChannelContractorAddressesTransaction>(
            command,
            mContractorsManager,
            mStorageHandler,
            mLog),
        true,
        false,
        false);
}

void TransactionsManager::launchSetChannelContractorCryptoKeyTransaction(
    SetChannelContractorCryptoKeyCommand::Shared command)
{
    prepareAndSchedule(
        make_shared<SetChannelContractorCryptoKeyTransaction>(
            command,
            mContractorsManager,
            mStorageHandler,
            mLog),
        true,
        false,
        true);
}

void TransactionsManager::launchRegenerateChannelCryptoKeyTransaction(
    RegenerateChannelCryptoKeyCommand::Shared command)
{
    prepareAndSchedule(
        make_shared<RegenerateChannelCryptoKeyTransaction>(
           command,
            mContractorsManager,
            mStorageHandler,
            mLog),
        true,
        false,
        false);
}

void TransactionsManager::launchUpdateChannelAddressesInitiatorTransaction()
{
    prepareAndSchedule(
        make_shared<UpdateChannelAddressesInitiatorTransaction>(
            mContractorsManager,
            mLog),
        true,
        false,
        true);
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
        command,
        mContractorsManager,
        mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
        mStorageHandler,
        mEventsInterface,
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
            command,
            mContractorsManager,
            trustLinesManager,
            mStorageHandler,
            mEquivalentsSubsystemsRouter->topologyCacheManager(command->equivalent()),
            mEquivalentsSubsystemsRouter->maxFlowCacheManager(command->equivalent()),
            mSubsystemsController,
            mKeysStore,
            mFeaturesManager,
            mEventsInterface,
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
            command,
            mContractorsManager,
            mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
            mStorageHandler,
            mEquivalentsSubsystemsRouter->topologyTrustLineManager(command->equivalent()),
            mEquivalentsSubsystemsRouter->topologyCacheManager(command->equivalent()),
            mEquivalentsSubsystemsRouter->maxFlowCacheManager(command->equivalent()),
            mSubsystemsController,
            mKeysStore,
            mFeaturesManager,
            mEventsInterface,
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
            message,
            mContractorsManager,
            mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
            mStorageHandler,
            mKeysStore,
            mEquivalentsSubsystemsRouter->topologyTrustLineManager(message->equivalent()),
            mEquivalentsSubsystemsRouter->topologyCacheManager(message->equivalent()),
            mEquivalentsSubsystemsRouter->maxFlowCacheManager(message->equivalent()),
            mFeaturesManager,
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

void TransactionsManager::launchRemoveTrustLineTransaction(
    RemoveTrustLineCommand::Shared command)
{
    try {
        auto transaction = make_shared<RemoveTrustLineTransaction>(
            command,
            mContractorsManager,
            mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
            mStorageHandler,
            mKeysStore,
            mLog);
        prepareAndSchedule(
            transaction,
            true,
            false,
            false);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for RemoveTrustLineTransaction "
                   "with equivalent " << command->equivalent() << " Details are: " << e.what();
    }
}

void TransactionsManager::launchResetTrustLineSourceTransaction(
    ResetTrustLineCommand::Shared command)
{
    try {
        auto transaction = make_shared<ResetTrustLineSourceTransaction>(
            command,
            mContractorsManager,
            mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
            mStorageHandler,
            mKeysStore,
            mLog);
        subscribeForAuditSignal(
            transaction->auditSignal);
        prepareAndSchedule(
            transaction,
            true,
            false,
            true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for ResetTrustLineSourceTransaction "
                   "with equivalent " << command->equivalent() << " Details are: " << e.what();
    }
}

void TransactionsManager::launchResetTrustLineDestinationTransaction(
    TrustLineResetMessage::Shared message)
{
    try {
        auto transaction = make_shared<ResetTrustLineDestinationTransaction>(
            message,
            mContractorsManager,
            mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
            mLog);
        prepareAndSchedule(
            transaction,
            false,
            false,
            true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for ResetTrustLineDestinationTransaction "
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
                command,
                mContractorsManager,
                mEquivalentsSubsystemsRouter,
                mTailManager,
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
                command,
                mContractorsManager,
                mEquivalentsSubsystemsRouter,
                mTailManager,
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
void TransactionsManager::launchMaxFlowCalculationSourceFstLevelTransaction(
    MaxFlowCalculationSourceFstLevelMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<MaxFlowCalculationSourceFstLevelTransaction>(
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
            isPaymentTransactionsAllowedDueToObserving,
            mEventsInterface,
            mLog,
            mSubsystemsController);
        subscribeForBuildCyclesThreeNodesTransaction(
            transaction->mBuildCycleThreeNodesSignal);
        subscribeForBuildCyclesFourNodesTransaction(
            transaction->mBuildCycleFourNodesSignal);
        subscribeForTrustLineActionSignal(
            transaction->trustLineActionSignal);
        subscribeForObserving(
            transaction);
        prepareAndSchedule(transaction, true, false, true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for CoordinatorPaymentTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
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
    if (!isPaymentTransactionsAllowedDueToObserving) {
        warning() << "It is forbid to run payment transactions due to observing";
        // todo : inform about fail by message
        return;
    }
    try {
        auto transaction = make_shared<ReceiverPaymentTransaction>(
            message,
            mEquivalentsSubsystemsRouter->iAmGateway(message->equivalent()),
            mContractorsManager,
            mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
            mStorageHandler,
            mEquivalentsSubsystemsRouter->topologyCacheManager(message->equivalent()),
            mEquivalentsSubsystemsRouter->maxFlowCacheManager(message->equivalent()),
            mResourcesManager,
            mKeysStore,
            mEventsInterface,
            mLog,
            mSubsystemsController);
        subscribeForBuildCyclesThreeNodesTransaction(
            transaction->mBuildCycleThreeNodesSignal);
        subscribeForBuildCyclesFourNodesTransaction(
            transaction->mBuildCycleFourNodesSignal);
        subscribeForTrustLineActionSignal(
            transaction->trustLineActionSignal);
        subscribeForObserving(
            transaction);
        prepareAndSchedule(transaction, false, false, true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for ReceiverPaymentTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
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
    if (!isPaymentTransactionsAllowedDueToObserving) {
        warning() << "It is forbid to run payment transactions due to observing";
        // todo : inform about fail by message
        return;
    }
    try {
        auto transaction = make_shared<IntermediateNodePaymentTransaction>(
            message,
            mEquivalentsSubsystemsRouter->iAmGateway(message->equivalent()),
            mContractorsManager,
            mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
            mStorageHandler,
            mEquivalentsSubsystemsRouter->topologyCacheManager(message->equivalent()),
            mEquivalentsSubsystemsRouter->maxFlowCacheManager(message->equivalent()),
            mResourcesManager,
            mKeysStore,
            mLog,
            mSubsystemsController);
        subscribeForBuildCyclesThreeNodesTransaction(
            transaction->mBuildCycleThreeNodesSignal);
        subscribeForBuildCyclesFourNodesTransaction(
            transaction->mBuildCycleFourNodesSignal);
        subscribeForTrustLineActionSignal(
            transaction->trustLineActionSignal);
        subscribeForObserving(
            transaction);
        prepareAndSchedule(transaction, false, false, true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for IntermediateNodePaymentTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
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
                message,
                mContractorsManager,
                mStorageHandler,
                mScheduler->isTransactionInProcess(
                    message->transactionUUID()),
                mSubsystemsController,
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }
}

void TransactionsManager::launchPaymentTransactionAfterGettingObservingSignatures(
    const TransactionUUID &transactionUUID,
    BlockNumber maximalClaimingBlockNumber,
    map<PaymentNodeID, lamport::Signature::Shared> participantsSignatures)
{
    info() << "launchPaymentTransactionAfterGettingObservingSignatures";
    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        auto serializedPaymentTransaction = ioTransaction->transactionHandler()->getTransaction(
            transactionUUID);
        auto *transactionType =
                new (serializedPaymentTransaction.get()) BaseTransaction::SerializedTransactionType;
        auto transactionTypeId = *transactionType;
        if (transactionTypeId != BaseTransaction::IntermediateNodePaymentTransaction and
                transactionTypeId != BaseTransaction::ReceiverPaymentTransaction and
                transactionTypeId != BaseTransaction::Payments_CycleCloserIntermediateNodeTransaction) {
            error() << "Transaction " << transactionUUID << " is not payment transaction";
            return;
        }
        auto *equivalent =
                new (serializedPaymentTransaction.get()
                     + sizeof(BaseTransaction::SerializedTransactionType)) SerializedEquivalent;
        auto paymentTransaction = deserializePaymentTransaction(
            serializedPaymentTransaction,
            transactionTypeId,
            *equivalent);
        if (paymentTransaction == nullptr) {
            error() << "Can't find deserialize payment TA: " << transactionUUID;
            return;
        }
        paymentTransaction->setObservingParticipantsSignatures(
            participantsSignatures);
        prepareAndSchedule(
            paymentTransaction,
            false,
            false,
            true);
    } catch (NotFoundError &e) {
        error() << "Can't find serialized TA: " << transactionUUID;
        return;
    }
}

void TransactionsManager::launchPaymentTransactionForObservingRejecting(
    const TransactionUUID &transactionUUID,
    BlockNumber maximalClaimingBlockNumber)
{
    info() << "launchPaymentTransactionAfterGettingObservingSignatures";
    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        auto serializedPaymentTransaction = ioTransaction->transactionHandler()->getTransaction(
            transactionUUID);
        auto *transactionType =
            new (serializedPaymentTransaction.get()) BaseTransaction::SerializedTransactionType;
        auto transactionTypeId = *transactionType;
        if (transactionTypeId != BaseTransaction::IntermediateNodePaymentTransaction and
            transactionTypeId != BaseTransaction::ReceiverPaymentTransaction and
            transactionTypeId != BaseTransaction::Payments_CycleCloserIntermediateNodeTransaction) {
            error() << "Transaction " << transactionUUID << " is not payment transaction";
            return;
        }
        auto *equivalent =
            new (serializedPaymentTransaction.get()
                     + sizeof(BaseTransaction::SerializedTransactionType)) SerializedEquivalent;
        auto paymentTransaction = deserializePaymentTransaction(
            serializedPaymentTransaction,
            transactionTypeId,
            *equivalent);
        if (paymentTransaction == nullptr) {
            error() << "Can't deserialize payment TA: " << transactionUUID;
            return;
        }
        paymentTransaction->setTransactionState(
            BasePaymentTransaction::Common_ObservingReject);
        prepareAndSchedule(
            paymentTransaction,
            false,
            false,
            true);
    } catch (NotFoundError &e) {
        error() << "Can't find serialized TA: " << transactionUUID;
        return;
    }
}

void TransactionsManager::launchPaymentTransactionForObservingUncertainStage(
    const TransactionUUID &transactionUUID,
    BlockNumber maximalClaimingBlockNumber)
{
    info() << "launchPaymentTransactionForObservingUncertainStage";
    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        auto serializedPaymentTransaction = ioTransaction->transactionHandler()->getTransaction(
            transactionUUID);
        auto *transactionType =
            new (serializedPaymentTransaction.get()) BaseTransaction::SerializedTransactionType;
        auto transactionTypeId = *transactionType;
        if (transactionTypeId != BaseTransaction::IntermediateNodePaymentTransaction and
            transactionTypeId != BaseTransaction::ReceiverPaymentTransaction and
            transactionTypeId != BaseTransaction::Payments_CycleCloserIntermediateNodeTransaction) {
            error() << "Transaction " << transactionUUID << " is not payment transaction";
            return;
        }
        auto *equivalent =
            new (serializedPaymentTransaction.get()
                 + sizeof(BaseTransaction::SerializedTransactionType)) SerializedEquivalent;
        auto paymentTransaction = deserializePaymentTransaction(
            serializedPaymentTransaction,
            transactionTypeId,
            *equivalent);
        if (paymentTransaction == nullptr) {
            error() << "Can't deserialize payment TA: " << transactionUUID;
            return;
        }
        paymentTransaction->setTransactionState(
            BasePaymentTransaction::Common_Uncertain);
        auto bytesAndCount = paymentTransaction->serializeToBytes();
        info() << "Transaction serialized";
        ioTransaction->transactionHandler()->saveRecord(
            transactionUUID,
            bytesAndCount.first,
            bytesAndCount.second);
        info() << "Transaction saved";
    } catch (NotFoundError &e) {
        error() << "Can't find serialized TA: " << transactionUUID;
        return;
    }
}

void TransactionsManager::launchCancelingPaymentTransaction(
    const TransactionUUID &transactionUUID,
    BlockNumber maximalClaimingBlockNumber)
{
    info() << "launchCancelingPaymentTransaction";
    // todo : develop transaction which will revert all changes which were made on during transaction transactionUUID
}

void TransactionsManager::onCloseCycleTransaction(
    const SerializedEquivalent equivalent,
    Path::Shared cycle)
{
    if (!isPaymentTransactionsAllowedDueToObserving) {
        warning() << "It is forbid to run payment transactions due to observing";
        return;
    }
    try {
        auto transaction = make_shared<CycleCloserInitiatorTransaction>(
            cycle,
            equivalent,
            mContractorsManager,
            mEquivalentsSubsystemsRouter->trustLinesManager(equivalent),
            mEquivalentsCyclesSubsystemsRouter->cyclesManager(equivalent),
            mStorageHandler,
            mEquivalentsSubsystemsRouter->topologyCacheManager(equivalent),
            mEquivalentsSubsystemsRouter->maxFlowCacheManager(equivalent),
            mResourcesManager,
            mKeysStore,
            mLog,
            mSubsystemsController);
        subscribeForTrustLineActionSignal(
            transaction->trustLineActionSignal);
        subscribeForObserving(
            transaction);
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
    if (!isPaymentTransactionsAllowedDueToObserving) {
        warning() << "It is forbid to run payment transactions due to observing";
        return;
    }
    try{
        auto transaction = make_shared<CycleCloserIntermediateNodeTransaction>(
            message,
            mContractorsManager,
            mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
            mEquivalentsCyclesSubsystemsRouter->cyclesManager(message->equivalent()),
            mStorageHandler,
            mEquivalentsSubsystemsRouter->topologyCacheManager(message->equivalent()),
            mEquivalentsSubsystemsRouter->maxFlowCacheManager(message->equivalent()),
            mResourcesManager,
            mKeysStore,
            mLog,
            mSubsystemsController);
        subscribeForTrustLineActionSignal(
            transaction->trustLineActionSignal);
        subscribeForObserving(
            transaction);
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
                equivalent,
                mContractorsManager,
                mEquivalentsSubsystemsRouter->trustLinesManager(equivalent),
                mEquivalentsCyclesSubsystemsRouter->cyclesManager(equivalent),
                mTailManager,
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
                equivalent,
                mContractorsManager,
                mEquivalentsSubsystemsRouter->trustLinesManager(equivalent),
                mEquivalentsCyclesSubsystemsRouter->cyclesManager(equivalent),
                mTailManager,
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
                command,
                mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
                mContractorsManager,
                mLog),
            true,
            false,
            false);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for GetFirstLevelContractorsTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
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
            make_shared<GetTrustLinesListTransaction>(
                command,
                mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
                mContractorsManager,
                mLog),
            true,
            false,
            false);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for GetTrustLinesListTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                command,
                mLog),
            false,
            false,
            false);
    }
}

void TransactionsManager::launchGetTrustLineByAddressTransaction(
    GetTrustLineByAddressCommand::Shared command)
{
    try {
        prepareAndSchedule(
            make_shared<GetTrustLineByAddressTransaction>(
                command,
                mContractorsManager,
                mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
                mLog),
            true,
            false,
            false);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for GetTrustLineByAddressTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                command,
                mLog),
            false,
            false,
            false);
    }
}

void TransactionsManager::launchGetTrustLineByIDTransaction(
    GetTrustLineByIDCommand::Shared command)
{
    try {
        prepareAndSchedule(
            make_shared<GetTrustLineByIDTransaction>(
                command,
                mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
                mLog),
            true,
            false,
            false);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for launchGetTrustLineByIDTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
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
            command,
            mEquivalentsSubsystemsRouter,
            mLog),
        true,
        false,
        false);
}

void TransactionsManager::launchPaymentTransactionByCommandUUIDTransaction(
    PaymentTransactionByCommandUUIDCommand::Shared command)
{
    try {
        prepareAndSchedule(
            make_shared<PaymentTransactionByCommandUUIDTransaction>(
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
                mContractorsManager,
                mEquivalentsSubsystemsRouter,
                mEquivalentsCyclesSubsystemsRouter.get(),
                mTailManager,
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

void TransactionsManager::launchFindPathByMaxFlowTransaction(
    const TransactionUUID &requestedTransactionUUID,
    BaseAddress::Shared destinationNodeAddress,
    const SerializedEquivalent equivalent)
{
    try {
        prepareAndSchedule(
            make_shared<FindPathByMaxFlowTransaction>(
                destinationNodeAddress,
                requestedTransactionUUID,
                equivalent,
                mContractorsManager,
                mResourcesManager,
                mEquivalentsSubsystemsRouter,
                mTailManager,
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

void TransactionsManager::allowPaymentTransactionsDueToObserving(
    bool allowPaymentTransactions)
{
    isPaymentTransactionsAllowedDueToObserving = allowPaymentTransactions;
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

void TransactionsManager::subscribeForObserving(
    BasePaymentTransaction::Shared transaction)
{
    transaction->observingClaimSignal.connect(
        boost::bind(
            &TransactionsManager::onObservingClaimReady,
            this,
            _1));


    transaction->mTransactionCommittedObservingSignal.connect(
        boost::bind(
            &TransactionsManager::onObservingTransactionCommitted,
            this,
            _1,
            _2));
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
    const ContractorID contractorID)
{
    transactionOutgoingMessageReadySignal(
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

void TransactionsManager::onObservingClaimReady(
    ObservingClaimAppendRequestMessage::Shared message)
{
    observingClaimSignal(message);
}

void TransactionsManager::onObservingTransactionCommitted(
    const TransactionUUID& transactionUUID,
    BlockNumber maxBlockNumberForClaiming)
{
    observingTransactionCommittedSignal(
        transactionUUID,
        maxBlockNumberForClaiming);
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
    processConfirmationMessageSignal(
        confirmationMessage);
}

void TransactionsManager::onProcessPongMessageSlot(
    ContractorID contractorID)
{
    processPongMessageSignal(
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
            contractorID,
            equivalent,
            mContractorsManager,
            trustLinesManager,
            mStorageHandler,
            mKeysStore,
            mFeaturesManager,
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
                equivalent,
                contractorID,
                mContractorsManager,
                mEquivalentsSubsystemsRouter->trustLinesManager(equivalent),
                mStorageHandler,
                mEventsInterface,
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
                equivalent,
                mContractorsManager,
                contractorID,
                mEquivalentsSubsystemsRouter->trustLinesManager(equivalent),
                mStorageHandler,
                mKeysStore,
                mFeaturesManager,
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
        case BaseTransaction::CoordinatorPaymentTransaction: {
            const auto kChildTransaction = static_pointer_cast<CoordinatorPaymentTransaction>(transaction);
            const auto transactionBytesAndCount = kChildTransaction->serializeToBytes();
            ioTransaction->transactionHandler()->saveRecord(
                kChildTransaction->currentTransactionUUID(),
                transactionBytesAndCount.first,
                transactionBytesAndCount.second);
            break;
        }
        case BaseTransaction::IntermediateNodePaymentTransaction: {
            const auto kChildTransaction = static_pointer_cast<IntermediateNodePaymentTransaction>(transaction);
            const auto transactionBytesAndCount = kChildTransaction->serializeToBytes();
            ioTransaction->transactionHandler()->saveRecord(
                kChildTransaction->currentTransactionUUID(),
                transactionBytesAndCount.first,
                transactionBytesAndCount.second);
            break;
        }
        case BaseTransaction::ReceiverPaymentTransaction: {
            const auto kChildTransaction = static_pointer_cast<ReceiverPaymentTransaction>(transaction);
            const auto transactionBytesAndCount = kChildTransaction->serializeToBytes();
            ioTransaction->transactionHandler()->saveRecord(
                kChildTransaction->currentTransactionUUID(),
                transactionBytesAndCount.first,
                transactionBytesAndCount.second);
            break;
        }
        case BaseTransaction::Payments_CycleCloserInitiatorTransaction: {
            const auto kChildTransaction = static_pointer_cast<CycleCloserInitiatorTransaction>(transaction);
            const auto transactionBytesAndCount = kChildTransaction->serializeToBytes();
            ioTransaction->transactionHandler()->saveRecord(
                kChildTransaction->currentTransactionUUID(),
                transactionBytesAndCount.first,
                transactionBytesAndCount.second);
            break;
        }
        case BaseTransaction::Payments_CycleCloserIntermediateNodeTransaction: {
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