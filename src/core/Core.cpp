#include "Core.h"

Core::Core(
    char* pArgv)
    noexcept:

    mCommandDescriptionPtr(pArgv)
{}

Core::~Core()
{}

int Core::run()
{
    auto initCode = initSubsystems();
    if (initCode != 0) {
        error() << "Core can't be initialised. Process will now be stopped.";
        return initCode;
    }

    writePIDFile();
    updateProcessName();

    try {
        mCommunicator->beginAcceptMessages();
        mCommandsInterface->beginAcceptCommands();

        info() << "Processing started.";
        mIOService.run();
        return 0;

    } catch (Exception &e) {
        mLog->logException("Core", e);
        return -1;
    }
}

int Core::initSubsystems()
{
    int initCode;

    initCode = initSettings();

    if (initCode != 0)
        return initCode;

    json conf;
    try {
        // Optimised conf read.
        // (several params at once)
        // For the details - see settings realisation.
        conf = mSettings->loadParsedJSON();

    } catch (std::exception &e) {
        cerr << utc_now() <<" : ERROR\tSETTINGS\t" <<  e.what() << "." << endl;
        return -1;
    }

    vector<SerializedEquivalent>equivalentsOnWhichIAmIsGateway;
    try {
        equivalentsOnWhichIAmIsGateway = mSettings->iAmGateway(&conf);
    } catch (RuntimeError &) {
        // Logger was not initialized yet
        cerr << utc_now() <<" : ERROR\tCORE\tCan't read if node is gateway from the settings" << endl;
        return -1;
    }

    initCode = initLogger();
    if (initCode != 0) {
        return initCode;
    }

    initCode = initStorageHandler();
    if (initCode != 0) {
        return initCode;
    }

    initCode = initContractorsManager(conf);
    if (initCode != 0) {
        return initCode;
    }

    initCode = initResourcesManager();
    if (initCode != 0) {
        return initCode;
    }

    initCode = initCommunicator();
    if (initCode != 0) {
        return initCode;
    }

    initCode = initObservingHandler(conf);
    if (initCode != 0) {
        return initCode;
    }

    initCode = initResultsInterface();
    if (initCode != 0) {
        return initCode;
    }

    initCode = initSubsystemsController();
    if (initCode != 0) {
        return initCode;
    }

    initCode = initTrustLinesInfluenceController();
    if (initCode != 0) {
        return initCode;
    }

    initCode = initKeysStore();
    if (initCode != 0) {
        return initCode;
    }

    initCode = initEquivalentsSubsystemsRouter(
        equivalentsOnWhichIAmIsGateway);
    if (initCode != 0) {
        return initCode;
    }

    initCode = initTransactionsManager();
    if (initCode != 0) {
        return initCode;
    }

    initCode = initCommandsInterface();
    if (initCode != 0) {
        return initCode;
    }

    connectSignalsToSlots();
    return 0;
}

int Core::initSettings()
{
    try {
        mSettings = make_unique<Settings>();
        // Logger was not initialized yet
        cerr << utc_now() <<" : SUCCESS\tCORE\tSettings are successfully initialised." << endl;
        return 0;

    } catch (const std::exception &e) {
        // Logger was not initialized yet
        cerr << utc_now() <<" : FATAL\tCORE\t" <<  e.what() << "." << endl;
        return -1;
    }
}

int Core::initLogger()
{
    try {
        mLog = make_unique<Logger>();
        return 0;

    } catch (...) {
        cerr << utc_now() <<" : FATAL\tCORE\tLogger cannot be initialized." << endl;
        return -1;
    }
}

int Core::initCommunicator()
{
    try {
        mCommunicator = make_unique<Communicator>(
            mIOService,
            mContractorsManager.get(),
            *mLog);

        info() << "Network communicator is successfully initialised";
        return 0;

    } catch (const std::exception &e) {
        mLog->logException("Core", e);
        return -1;
    }
}

int Core::initObservingHandler(
    const json &conf)
{
    try {
        mObservingHandler = make_unique<ObservingHandler>(
            mSettings->observers(&conf),
            mIOService,
            mStorageHandler.get(),
            mResourcesManager.get(),
            *mLog);

        info() << "Observing handler is successfully initialised";
        return 0;

    } catch (const std::exception &e) {
        mLog->logException("Core", e);
        return -1;
    }
}

int Core::initResultsInterface()
{
    try {
        mResultsInterface = make_unique<ResultsInterface>(
            *mLog);
        info() << "Results interface is successfully initialised";
        return 0;

    } catch (const std::exception &e) {
        mLog->logException("Core", e);
        return -1;
    }
}

int Core::initEquivalentsSubsystemsRouter(
    vector<SerializedEquivalent> equivalentIAmGateway)
{
    try {
        mEquivalentsSubsystemsRouter = make_unique<EquivalentsSubsystemsRouter>(
            mStorageHandler.get(),
            mKeysStore.get(),
            mContractorsManager.get(),
            mIOService,
            equivalentIAmGateway,
            *mLog);
        info() << "EquivalentsSubsystemsRouter is successfully initialised";
        return 0;
    } catch (const std::exception &e) {
        mLog->logException("Core", e);
        return -1;
    }

}

int Core::initResourcesManager()
{
    try {
        mResourcesManager = make_unique<ResourcesManager>();
        info() << "Resources manager is successfully initialized";
        return 0;
    } catch (const std::exception &e) {
        mLog->logException("Core", e);
        return -1;
    }
}

int Core::initTransactionsManager()
{
    try {
        mTransactionsManager = make_unique<TransactionsManager>(
            mIOService,
            mContractorsManager.get(),
            mEquivalentsSubsystemsRouter.get(),
            mResourcesManager.get(),
            mResultsInterface.get(),
            mStorageHandler.get(),
            mKeysStore.get(),
            *mLog,
            mSubsystemsController.get(),
            mTrustLinesInfluenceController.get());
        info() << "Transactions handler is successfully initialised";
        return 0;

    } catch (const std::exception &e) {
        mLog->logException("Core", e);
        return -1;
    }
}

int Core::initCommandsInterface()
{
    try {
        mCommandsInterface = make_unique<CommandsInterface>(
            mIOService,
            *mLog);
        info() << "Commands interface is successfully initialised";
        return 0;

    } catch (const std::exception &e) {
        mLog->logException("Core", e);
        return -1;
    }
}

int Core::initStorageHandler()
{
    try {
        mStorageHandler = make_unique<StorageHandler>(
            "io",
            "storageDB",
            *mLog);
        info() << "Storage handler is successfully initialised";
        return 0;
    } catch (const std::exception &e) {
        mLog->logException("Core", e);
        return -1;
    }
}

int Core::initContractorsManager(
    const json &conf)
{
    try {
        mContractorsManager = make_unique<ContractorsManager>(
            mSettings->addresses(&conf),
            mStorageHandler.get(),
            *mLog);
        info() << "Contractors manager is successfully initialised";
        return 0;
    } catch (const std::exception &e) {
        mLog->logException("Core", e);
        return -1;
    }
}

int Core::initSubsystemsController()
{
    try {
        mSubsystemsController = make_unique<SubsystemsController>(
            *mLog);
        info() << "Subsystems controller is successfully initialized";
        return 0;
    } catch (const std::exception &e) {
        mLog->logException("Core", e);
        return -1;
    }
}

int Core::initTrustLinesInfluenceController()
{
    try {
        mTrustLinesInfluenceController = make_unique<TrustLinesInfluenceController>(
            *mLog);
        info() << "Trust Lines Influence controller is successfully initialized";
        return 0;
    } catch (const std::exception &e) {
        mLog->logException("Core", e);
        return -1;
    }
}

int Core::initKeysStore()
{
    try {
        mKeysStore = make_unique<Keystore>(
            *mLog);
        mKeysStore->init();
        info() << "Keys store is successfully initialized";
        return 0;
    } catch (const std::exception &e) {
        mLog->logException("Core", e);
        return -1;
    }
}

void Core::connectCommandsInterfaceSignals ()
{
    mCommandsInterface->commandReceivedSignal.connect(
        boost::bind(
            &Core::onCommandReceivedSlot,
            this,
            _1));
}

void Core::connectCommunicatorSignals()
{
    //communicator's signal to transactions manager slot
    mCommunicator->signalMessageReceived.connect(
        boost::bind(
            &Core::onMessageReceivedSlot,
            this,
            _1));

    mCommunicator->signalClearTopologyCache.connect(
        boost::bind(
            &Core::onClearTopologyCacheSlot,
            this,
            _1,
            _2));

    //transactions manager's to communicator slot
    mTransactionsManager->transactionOutgoingMessageReadySignal.connect(
        boost::bind(
            &Core::onMessageSendSlot,
            this,
            _1,
            _2));

    mTransactionsManager->transactionOutgoingMessageToAddressReadySignal.connect(
        boost::bind(
            &Core::onMessageSendToAddressSlot,
            this,
            _1,
            _2));

    mTransactionsManager->transactionOutgoingMessageWithCachingReadySignal.connect(
        boost::bind(
            &Core::onMessageSendWithCachingSlot,
            this,
            _1,
            _2,
            _3,
            _4));

    mTransactionsManager->processConfirmationMessageSignal.connect(
        boost::bind(
            &Core::onProcessConfirmationMessageSlot,
            this,
            _1));

    mTransactionsManager->processPongMessageSignal.connect(
        boost::bind(
            &Core::onProcessPongMessageSlot,
            this,
            _1));

    for (const auto &contractorID : mEquivalentsSubsystemsRouter->contractorsShouldBePinged()) {
        mCommunicator->enqueueContractorWithPostponedSending(
            contractorID);
    }
    mEquivalentsSubsystemsRouter->clearContractorsShouldBePinged();
}

void Core::connectObservingSignals()
{
    mTransactionsManager->observingClaimSignal.connect(
        boost::bind(
            &Core::onMessageSendToObserverSlot,
            this,
            _1));

    mTransactionsManager->observingTransactionCommittedSignal.connect(
        boost::bind(
            &Core::onAddTransactionToObservingCheckingSlot,
            this,
            _1,
            _2));

    mObservingHandler->mParticipantsVotesSignal.connect(
        boost::bind(
            &Core::onObservingParticipantsVotesSlot,
            this,
            _1,
            _2,
            _3));

    mObservingHandler->mRejectTransactionSignal.connect(
        boost::bind(
            &Core::onObservingTransactionRejectSlot,
            this,
            _1,
            _2));
}

void Core::connectResourcesManagerSignals()
{
    mResourcesManager->requestPathsResourcesSignal.connect(
        boost::bind(
            &Core::onPathsResourceRequestedSlot,
            this,
            _1,
            _2,
            _3));

    mResourcesManager->requestObservingBlockNumberSignal.connect(
        boost::bind(
            &Core::onObservingBlockNumberRequestSlot,
            this,
            _1));

    mResourcesManager->attachResourceSignal.connect(
        boost::bind(
            &Core::onResourceCollectedSlot,
            this,
            _1));
}
void Core::connectSignalsToSlots()
{
    connectCommandsInterfaceSignals();
    connectCommunicatorSignals();
    connectResourcesManagerSignals();
    connectObservingSignals();
}

void Core::onCommandReceivedSlot (
    BaseUserCommand::Shared command)
{
    if (command->identifier() == SubsystemsInfluenceCommand::identifier()) {
        // In case if network toggle command was received -
        // there is no reason to transfer it's processing to the transactions manager:
        // this command only enables or disables network for the node,
        // and this may be simply done by filtering several slots in the core.
        auto subsystemsInfluenceCommand = static_pointer_cast<SubsystemsInfluenceCommand>(command);
        mSubsystemsController->setFlags(
            subsystemsInfluenceCommand->flags());
#ifdef TESTS
        mSubsystemsController->setForbiddenNodeUUID(
            subsystemsInfluenceCommand->forbiddenNodeUUID());
        mSubsystemsController->setForbiddenAmount(
            subsystemsInfluenceCommand->forbiddenAmount());
        // set node as gateway
        if ((subsystemsInfluenceCommand->flags() & 0x80000000000) != 0) {
            info() << "from now I am gateway";
            mEquivalentsSubsystemsRouter->setMeAsGateway();
        }
#endif
        info() << "SubsystemsInfluenceCommand processed";
        return;
    }

#ifdef TESTS
    if (command->identifier() == TrustLinesInfluenceCommand::identifier()) {
        auto trustLinesInfluenceCommand = static_pointer_cast<TrustLinesInfluenceCommand>(command);
        mTrustLinesInfluenceController->setFlags(trustLinesInfluenceCommand->flags());
        mTrustLinesInfluenceController->setFirstParameter(
            trustLinesInfluenceCommand->firstParameter());
        mTrustLinesInfluenceController->setSecondParameter(
            trustLinesInfluenceCommand->secondParameter());
        mTrustLinesInfluenceController->setThirdParameter(
            trustLinesInfluenceCommand->thirdParameter());
        return;
    }
#endif

    try {
        mTransactionsManager->processCommand(command);

    } catch(exception &e) {
        mLog->logException("Core", e);
    }
}

void Core::onMessageReceivedSlot(
    Message::Shared message)
{
#ifdef TESTS
    if (not mSubsystemsController->isNetworkOn()) {
        // Ignore incoming message in case if network was disabled.
        debug() << "Ignore process incoming message";
        return;
    }
#endif

#ifdef TESTS
    if (mTrustLinesInfluenceController->checkReceivedMessage(message->typeID())) {
        // Ignore incoming message of forbidden type
        debug() << "Ignore processing incoming message of forbidden type " << message->typeID();
        return;
    }
#endif

    try {
        mTransactionsManager->processMessage(message);

    } catch(exception &e) {
        mLog->logException("Core", e);
    }
}

void Core::onClearTopologyCacheSlot(
    const SerializedEquivalent equivalent,
    BaseAddress::Shared nodeAddress)
{
    try {
        mEquivalentsSubsystemsRouter->topologyCacheManager(equivalent)->removeCache(nodeAddress);
    } catch (NotFoundError &e) {
        error() << "There are no topologyCacheManager for onClearTopologyCacheSlot "
                "with equivalent " << equivalent << " Details are: " << e.what();
    }
}

void Core::onMessageSendSlot(
    Message::Shared message,
    const ContractorID contractorID)
{
#ifdef TESTS
    if (not mSubsystemsController->isNetworkOn()) {
        // Ignore outgoing message in case if network was disabled.
        debug() << "Ignore send message";
        return;
    }
#endif

    try {
        mCommunicator->sendMessage(
            message,
            contractorID);

    } catch (exception &e) {
        mLog->logException("Core", e);
    }
}

void Core::onMessageSendToAddressSlot(
    Message::Shared message,
    BaseAddress::Shared address)
{
#ifdef TESTS
    if (not mSubsystemsController->isNetworkOn()) {
        // Ignore outgoing message in case if network was disabled.
        debug() << "Ignore send message";
        return;
    }
#endif

    try {
        mCommunicator->sendMessage(
            message,
            address);

    } catch (exception &e) {
        mLog->logException("Core", e);
    }
}

void Core::onMessageSendWithCachingSlot(
    TransactionMessage::Shared message,
    ContractorID contractorID,
    Message::MessageType incomingMessageTypeFilter,
    uint32_t cacheTimeLiving)
{
#ifdef TESTS
    if (not mSubsystemsController->isNetworkOn()) {
        // Ignore outgoing message in case if network was disabled.
        debug() << "Ignore send message";
        return;
    }
#endif

    try {
        mCommunicator->sendMessageWithCacheSaving(
            message,
            contractorID,
            incomingMessageTypeFilter,
            cacheTimeLiving);

    } catch (exception &e) {
        mLog->logException("Core", e);
    }
}

void Core::onMessageSendToObserverSlot(
    ObservingClaimAppendRequestMessage::Shared message)
{
    mObservingHandler->sendClaimRequestToObservers(message);
}

void Core::onAddTransactionToObservingCheckingSlot(
    const TransactionUUID& transactionUUID,
    BlockNumber maxBlockNumberForClaiming)
{
    mObservingHandler->addTransactionForChecking(
        transactionUUID,
        maxBlockNumberForClaiming);
}

void Core::onObservingParticipantsVotesSlot(
    const TransactionUUID &transactionUUID,
    BlockNumber maximalClaimingBlockNumber,
    map<PaymentNodeID, lamport::Signature::Shared> participantsSignatures)
{
    mTransactionsManager->launchPaymentTransactionAfterGettingObservingSignatures(
        transactionUUID,
        maximalClaimingBlockNumber,
        participantsSignatures);
}

void Core::onObservingTransactionRejectSlot(
    const TransactionUUID &transactionUUID,
    BlockNumber maximalClaimingBlockNumber)
{
    mTransactionsManager->launchPaymentTransactionForObservingRejecting(
        transactionUUID,
        maximalClaimingBlockNumber);
}

void Core::onPathsResourceRequestedSlot(
    const TransactionUUID &transactionUUID,
    BaseAddress::Shared destinationNodeAddress,
    const SerializedEquivalent equivalent)
{
    try {
        mTransactionsManager->launchFindPathByMaxFlowTransaction(
            transactionUUID,
            destinationNodeAddress,
            equivalent);

    } catch (exception &e) {
        mLog->logException("Core", e);
    }
}

void Core::onObservingBlockNumberRequestSlot(
    const TransactionUUID &transactionUUID)
{
    mObservingHandler->requestActualBlockNumber(
        transactionUUID);
}

void Core::onResourceCollectedSlot(
    BaseResource::Shared resource)
{
    try {
        mTransactionsManager->attachResourceToTransaction(
            resource);

    } catch (exception &e) {
        mLog->logException("Core", e);
    }
}

void Core::onProcessConfirmationMessageSlot(
    ConfirmationMessage::Shared confirmationMessage)
{
    mCommunicator->processConfirmationMessage(
        confirmationMessage);
}

void Core::onProcessPongMessageSlot(
    ContractorID contractorID)
{
    mCommunicator->processPongMessage(
        contractorID);
}

void Core::writePIDFile()
{
    try {
        std::ofstream pidFile("process.pid");
        pidFile << ::getpid() << std::endl;
        pidFile.close();

    } catch (std::exception &e) {
        error() << "Can't write/update pid file. Error message is: " << e.what();
    }
}

void Core::updateProcessName()
{
    const string kProcessName(string("GEO:") + mContractorsManager->selfContractor()->mainAddress()->fullAddress());
    prctl(PR_SET_NAME, kProcessName.c_str());
    strcpy(mCommandDescriptionPtr, kProcessName.c_str());
}

string Core::logHeader()
    noexcept
{
    return "[CORE]";
}

LoggerStream Core::error() const
    noexcept
{
    return mLog->error(logHeader());
}

LoggerStream Core::warning() const
    noexcept
{
    return mLog->warning(logHeader());
}

LoggerStream Core::info() const
    noexcept
{
    return mLog->info(logHeader());
}

LoggerStream Core::debug() const
    noexcept
{
    return mLog->debug(logHeader());
}
