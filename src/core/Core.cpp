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
        if (!mCommunicator->joinUUID2Address(mNodeUUID)) {
            error() << "Core can't be initialised. Process will now be stopped.";
            return -1;
        }
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

    try {
        mNodeUUID = mSettings->nodeUUID(&conf);

    } catch (RuntimeError &) {
        // Logger was not initialized yet
        cerr << utc_now() <<" : ERROR\tCORE\tCan't read UUID of the node from the settings" << endl;
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

    initCode = initCommunicator(conf);
    if (initCode != 0) {
        return initCode;
    }

    initCode = initResultsInterface();
    if (initCode != 0) {
        return initCode;
    }

    initCode = initStorageHandler();
    if (initCode != 0) {
        return initCode;
    }

    initCode = initResourcesManager();
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

    initCode = initEquivalentsSubsystemsRouter(
        equivalentsOnWhichIAmIsGateway);
    if (initCode != 0) {
        return initCode;
    }

    initCode = initKeysStore();
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
        mLog = make_unique<Logger>(mNodeUUID);
        return 0;

    } catch (...) {
        cerr << utc_now() <<" : FATAL\tCORE\tLogger cannot be initialized." << endl;
        return -1;
    }
}

int Core::initCommunicator(
    const json &conf)
{
    try {
        mCommunicator = make_unique<Communicator>(
            mIOService,
            mSettings->interface(&conf),
            mSettings->port(&conf),
            mSettings->uuid2addressHost(&conf),
            mSettings->uuid2addressPort(&conf),
            mNodeUUID,
            *mLog);

        info() << "Network communicator is successfully initialised";
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
            mNodeUUID,
            mStorageHandler.get(),
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
            mNodeUUID,
            mIOService,
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

    mTransactionsManager->transactionOutgoingMessageWithCachingReadySignal.connect(
        boost::bind(
            &Core::onMessageSendWithCachingSlot,
            this,
            _1,
            _2,
            _3));

    mTransactionsManager->ProcessConfirmationMessageSignal.connect(
        boost::bind(
            &Core::onProcessConfirmationMessageSlot,
            this,
            _1));
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
        if (mSubsystemsController->isWriteVisualResults()) {
            mTransactionsManager->activateVisualInterface();
        } else {
            mTransactionsManager->deactivateVisualInterface();
        }
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
        mTrustLinesInfluenceController->setForbiddenReceiveMessageType(
            trustLinesInfluenceCommand->forbiddenReceiveMessageType());
        mTrustLinesInfluenceController->setCountForbiddenReceivedMessages(
            trustLinesInfluenceCommand->countForbiddenReceivedMessages());
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
    const NodeUUID &nodeUUID)
{
    try {
        mEquivalentsSubsystemsRouter->topologyCacheManager(equivalent)->removeCache(nodeUUID);
    } catch (NotFoundError &e) {
        error() << "There are no topologyCacheManager for onClearTopologyCacheSlot "
                "with equivalent " << equivalent << " Details are: " << e.what();
    }
}

void Core::onMessageSendSlot(
    Message::Shared message,
    const NodeUUID &contractorUUID)
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
            contractorUUID);

    } catch (exception &e) {
        mLog->logException("Core", e);
    }
}

void Core::onMessageSendWithCachingSlot(
    TransactionMessage::Shared message,
    const NodeUUID &contractorUUID,
    Message::MessageType incomingMessageTypeFilter)
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
            contractorUUID,
            incomingMessageTypeFilter);

    } catch (exception &e) {
        mLog->logException("Core", e);
    }
}

void Core::onPathsResourceRequestedSlot(
    const TransactionUUID &transactionUUID,
    const NodeUUID &destinationNodeUUID,
    const SerializedEquivalent equivalent)
{
    try {
        mTransactionsManager->launchFindPathByMaxFlowTransaction(
            transactionUUID,
            destinationNodeUUID,
            equivalent);

    } catch (exception &e) {
        mLog->logException("Core", e);
    }
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
    const string kProcessName(string("GEO:") + mNodeUUID.stringUUID());
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
