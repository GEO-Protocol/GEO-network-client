#include "Core.h"

Core::Core()
{
}

Core::~Core()
{}

int Core::run()
{


    auto initCode = initSubsystems();
    writePIDFile();
    if (initCode != 0) {
        mLog->logFatal("Core", "Can't be initialised. Process will now be stopped.");
        return initCode;
    }

    try {
        mCommunicator->joinUUID2Address(mNodeUUID);
        mCommunicator->beginAcceptMessages();
        mCommandsInterface->beginAcceptCommands();

        mLog->logSuccess("Core", "Processing started.");
        mIOService.run();
        return 0;

    } catch (Exception &e) {
        mLog->logException("Core", e);
        return -1;
    }

}

int Core::initSubsystems() {

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
        mLog->logException("Settings", e);
        return -1;
    }

    try {
        mNodeUUID = mSettings->nodeUUID(&conf);

    } catch (RuntimeError &) {
        // todo what to do if settings cannot initiaize
//        mLog->logFatal("Core", "Can't read UUID of the node from the settings.");
        return -1;
    }

    initCode = initLogger(conf);
    if (initCode != 0)
        return initCode;

    initCode = initCommunicator(conf);
    if (initCode != 0)
        return initCode;

    initCode = initResultsInterface();
    if (initCode != 0)
        return initCode;

    initCode = initStorageHandler();
    if (initCode != 0)
        return initCode;

    initCode = initTrustLinesManager();
    if (initCode != 0)
        return initCode;

    initCode = initMaxFlowCalculationTrustLineManager();
    if (initCode != 0)
        return initCode;

    initCode = initMaxFlowCalculationCacheManager();
    if (initCode != 0) {
        return initCode;
    }

    initCode = initPathsManager();
    if (initCode != 0) {
        return initCode;
    }

    initCode = initResourcesManager();
    if (initCode != 0) {
        return initCode;
    }

    initCode = initTransactionsManager();
    if (initCode != 0)
        return initCode;

    initCode = initCommandsInterface();
    if (initCode != 0)
        return initCode;

    initCode = initDelayedTasks();
    if (initCode != 0)
        return initCode;

    connectSignalsToSlots();

    // TODO: Remove me
    // This scheme is needd for payments tests
    // Please, do no remove it untile payments would be done

//    if (mNodeUUID.stringUUID() == string("13e5cf8c-5834-4e52-b65b-f9281dd1ff00")) {
//        mTrustLinesManager->accept(NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff01"), TrustLineAmount(100));
//
//    } else if (mNodeUUID.stringUUID() == string("13e5cf8c-5834-4e52-b65b-f9281dd1ff01")) {
//        mTrustLinesManager->open(NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff00"), TrustLineAmount(100));
//        mTrustLinesManager->accept(NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff02"), TrustLineAmount(90));
//
//    } else if (mNodeUUID.stringUUID() == string("13e5cf8c-5834-4e52-b65b-f9281dd1ff02")) {
//        mTrustLinesManager->open(NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff01"), TrustLineAmount(90));
//        mTrustLinesManager->accept(NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff03"), TrustLineAmount(80));
//
//    } else if (mNodeUUID.stringUUID() == string("13e5cf8c-5834-4e52-b65b-f9281dd1ff03")) {
//        mTrustLinesManager->open(NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff02"), TrustLineAmount(80));
//    }

    return 0;
}

int Core::initSettings() {

    try {
        mSettings = make_unique<Settings>();
        // mLog.logSuccess("Core", "Settings are successfully initialised");
        return 0;

    } catch (const std::exception &e) {
        //  mLog.logException("Core", e);
        return -1;
    }
}

int Core::initLogger(
    const json &conf)
{
    try {
        mLog = make_unique<Logger>(
            mNodeUUID,
            mIOService,
            mSettings->influxDbHost(&conf),
            mSettings->influxDbName(&conf),
            mSettings->influxDbPort(&conf)
        );
        return 0;
    } catch (const std::exception &e) {
        // todo add notify that loger canot be initialize
    //        mLog.logException("Core", e);
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
            *mLog.get());

        mLog->logSuccess("Core", "Network communicator is successfully initialised");
        return 0;

    } catch (const std::exception &e) {
        mLog->logException("Core", e);
        return -1;
    }
}

int Core::initResultsInterface() {

    try {
        mResultsInterface = make_unique<ResultsInterface>(
            *mLog.get());
        mLog->logSuccess("Core", "Results interface is successfully initialised");
        return 0;

    } catch (const std::exception &e) {
        mLog->logException("Core", e);
        return -1;
    }
}

int Core::initTrustLinesManager() {

    try {
        mTrustLinesManager = make_unique<TrustLinesManager>(
            mStorageHandler.get(),
            *mLog.get());
        mLog->logSuccess("Core", "Trust lines manager is successfully initialised");
        return 0;

    }catch(const std::exception &e) {
        mLog->logException("Core", e);
        return -1;
    }
}

int Core::initMaxFlowCalculationTrustLineManager() {

    try{
        mMaxFlowCalculationTrustLimeManager = make_unique<MaxFlowCalculationTrustLineManager>(
            *mLog.get());
        mLog->logSuccess("Core", "Max flow calculation Trust lines manager is successfully initialised");
        return 0;

    }catch(const std::exception &e) {
        mLog->logException("Core", e);
        return -1;
    }
}

int Core::initMaxFlowCalculationCacheManager() {

    try {
        mMaxFlowCalculationCacheManager = make_unique<MaxFlowCalculationCacheManager>(
            *mLog.get());
        mLog->logSuccess("Core", "Max flow calculation Cache manager is successfully initialised");
        return 0;

    } catch (const std::exception &e) {
        mLog->logException("Core", e);
        return -1;
    }
}

int Core::initResourcesManager() {

    try {
        mResourcesManager = make_unique<ResourcesManager>();
        mLog->logSuccess("Core", "Resources manager is successfully initialized");
        return 0;
    } catch (const std::exception &e) {
        mLog->logException("Core", e);
        return -1;
    }
}

int Core::initTransactionsManager() {

    try {
        mTransactionsManager = make_unique<TransactionsManager>(
            mNodeUUID,
            mIOService,
            mTrustLinesManager.get(),
            mResourcesManager.get(),
            mMaxFlowCalculationTrustLimeManager.get(),
            mMaxFlowCalculationCacheManager.get(),
            mResultsInterface.get(),
            mStorageHandler.get(),
            mPathsManager.get(),
            *mLog.get());
        mLog->logSuccess("Core", "Transactions handler is successfully initialised");
        return 0;

    } catch (const std::exception &e) {
        mLog->logException("Core", e);
        return -1;
    }
}

int Core::initDelayedTasks() {
    try{
        mMaxFlowCalculationCacheUpdateDelayedTask = make_unique<MaxFlowCalculationCacheUpdateDelayedTask>(
                mIOService,
                mMaxFlowCalculationCacheManager.get(),
                mMaxFlowCalculationTrustLimeManager.get(),
                *mLog.get());
        mLog->logSuccess("Core", "DelayedTasks is successfully initialised");
        return 0;
    } catch (const std::exception &e) {
        mLog->logException("Core", e);
        return -1;
    }
}

int Core::initCommandsInterface() {

    try {
        mCommandsInterface = make_unique<CommandsInterface>(
            mIOService,
            *mLog.get());
        mLog->logSuccess("Core", "Commands interface is successfully initialised");
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

int Core::initStorageHandler() {

    try {
        mStorageHandler = make_unique<StorageHandler>(
            "io",
            "storageDB",
            *mLog.get());
        mLog->logSuccess("Core", "Storage handler is successfully initialised");
        return 0;
    } catch (const std::exception &e) {
        mLog->logException("Core", e);
        return -1;
    }
}

int Core::initPathsManager() {

    try {
        mPathsManager = make_unique<PathsManager>(
            mNodeUUID,
            mTrustLinesManager.get(),
            mStorageHandler.get(),
            *mLog.get());
        mLog->logSuccess("Core", "Paths Manager is successfully initialised");
        return 0;
    } catch (const std::exception &e) {
        mLog->logException("Core", e);
        return -1;
    }
}

void Core::connectCommunicatorSignals() {

    //communicator's signal to transactions manager slot
    mCommunicator->signalMessageReceived.connect(
        boost::bind(
            &Core::onMessageReceivedSlot,
            this,
            _1
        )
    );

    //transactions manager's to communicator slot
    mTransactionsManager->transactionOutgoingMessageReadySignal.connect(
        boost::bind(
            &Core::onMessageSendSlot,
            this,
            _1,
            _2
        )
    );
}
void Core::connectTrustLinesManagerSignals() {

    mTrustLinesManager->trustLineCreatedSignal.connect(
        boost::bind(
            &Core::onTrustLineCreatedSlot,
            this,
            _1,
            _2
        )
    );

    mTrustLinesManager->trustLineStateModifiedSignal.connect(
        boost::bind(
            &Core::onTrustLineStateModifiedSlot,
            this,
            _1,
            _2
        )
    );
}

void Core::connectResourcesManagerSignals() {

    mResourcesManager->requestPathsResourcesSignal.connect(
        boost::bind(
            &Core::onPathsResourceRequestedSlot,
            this,
            _1,
            _2
        )
    );

    mResourcesManager->attachResourceSignal.connect(
        boost::bind(
            &Core::onResourceCollectedSlot,
            this,
            _1
        )
    );
}
void Core::connectSignalsToSlots()
{
    connectCommandsInterfaceSignals();
    connectCommunicatorSignals();
    connectTrustLinesManagerSignals();
    connectResourcesManagerSignals();
}

void Core::onCommandReceivedSlot (
    BaseUserCommand::Shared command)
{
    try {
        mTransactionsManager->processCommand(command);

    } catch(exception &e) {
        mLog->logException("Core", e);
    }
}

void Core::onMessageReceivedSlot(
    Message::Shared message) {

    try {
        mTransactionsManager->processMessage(message);

    } catch(exception &e) {
        mLog->logException("Core", e);
    }
}

void Core::onMessageSendSlot(
    Message::Shared message,
    const NodeUUID &contractorUUID) {

    try{
        mCommunicator->sendMessage(
            message,
            contractorUUID
        );

    } catch (exception &e) {
        mLog->logException("Core", e);
    }
}

void Core::onTrustLineCreatedSlot(
    const NodeUUID &contractorUUID,
    const TrustLineDirection direction) {

}

void Core::onTrustLineStateModifiedSlot(
    const NodeUUID &contractorUUID,
    const TrustLineDirection direction) {

}

void Core::onPathsResourceRequestedSlot(
    const TransactionUUID &transactionUUID,
    const NodeUUID &destinationNodeUUID) {
    try {
        mTransactionsManager->launchPathsResourcesCollectTransaction(
            transactionUUID,
            destinationNodeUUID);

    } catch (exception &e) {
        mLog->logException("Core", e);
    }

}

void Core::onResourceCollectedSlot(
    BaseResource::Shared resource) {

    try {
        mTransactionsManager->attachResourceToTransaction(
            resource);

    } catch (exception &e) {
        mLog->logException("Core", e);
    }

}

void Core::writePIDFile()
{
    try {
        std::ofstream pidFile("process.pid");
        pidFile << ::getpid() << std::endl;
        pidFile.close();

    } catch (std::exception &e) {
        auto errors = mLog->error("Core");
        errors << "Can't write/update pid file. Error message is: " << e.what();
    }
}

void Core::checkSomething() {
    printRTs();
}

void Core::printRTs() {
    cout << "Cyurrent UUID" << mNodeUUID << endl;
    cout  << "printRTs\tRT1 size: " << mTrustLinesManager->trustLines().size() << endl;
    for (const auto itTrustLine : mTrustLinesManager->trustLines()) {
        cout  << "printRTs\t" << itTrustLine.second->contractorNodeUUID() << " "
        << itTrustLine.second->incomingTrustAmount() << " "
        << itTrustLine.second->outgoingTrustAmount() << " "
        << itTrustLine.second->balance() << endl;
    }
    cout  << "printRTs\tRT2 size: " << mStorageHandler->routingTablesHandler()->rt2Records().size() << endl;
    for (auto const itRT2 : mStorageHandler->routingTablesHandler()->rt2Records()) {
        cout  << itRT2.first << " " << itRT2.second << endl;
    }
    cout  << "printRTs\tRT3 size: " << mStorageHandler->routingTablesHandler()->rt3Records().size() << endl;
    for (auto const itRT3 : mStorageHandler->routingTablesHandler()->rt3Records()) {
        cout  << itRT3.first << " " << itRT3.second << endl;
    }
}
