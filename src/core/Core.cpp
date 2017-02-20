#include "Core.h"

Core::Core() {

    zeroPointers();
}

Core::~Core() {

    cleanupMemory();
}

int Core::run() {

    auto initCode = initCoreComponents();
    if (initCode != 0) {
        mLog.logFatal("Core", "Core components can't be initialised. Process will now be closed.");
        return initCode;
    }
    JustToTestSomething();
    try {
        mCommunicator->beginAcceptMessages();
        mCommandsInterface->beginAcceptCommands();

        mLog.logSuccess("Core", "Processing started.");
        mIOService.run();
        return 0;

    } catch (const std::exception &e) {
        mLog.logException("Core", e);
        return -1;
    }

}

int Core::initCoreComponents() {

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
        mLog.logException("Settings", e);
        return -1;
    }

    try {
        mNodeUUID = mSettings->nodeUUID(&conf);

    } catch (RuntimeError &) {
        mLog.logFatal("Core", "Can't read transactionUUID of the node from the settings.");
        return -1;
    }

    initCode = initCommunicator(conf);
    if (initCode != 0)
        return initCode;

    initCode = initResultsInterface();
    if (initCode != 0)
        return initCode;

    initCode = initTrustLinesManager();
    if (initCode != 0)
        return initCode;

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

    return 0;
}

int Core::initSettings() {

    try {
        mSettings = new Settings();
        mLog.logSuccess("Core", "Settings are successfully initialised");
        return 0;

    } catch (const std::exception &e) {
        mLog.logException("Core", e);
        return -1;
    }
}

int Core::initCommunicator(
    const json &conf) {

    try {
        mCommunicator = new Communicator(
            mIOService,
            mNodeUUID,
            mSettings->interface(&conf),
            mSettings->port(&conf),
            mSettings->uuid2addressHost(&conf),
            mSettings->uuid2addressPort(&conf),
            &mLog
        );
        mLog.logSuccess("Core", "NetworkSlots communicator is successfully initialised");
        return 0;

    } catch (const std::exception &e) {
        mLog.logException("Core", e);
        return -1;
    }
}

int Core::initResultsInterface() {

    try {
        mResultsInterface = new ResultsInterface(&mLog);
        mLog.logSuccess("Core", "Results interface is successfully initialised");
        return 0;

    } catch (const std::exception &e) {
        mLog.logException("Core", e);
        return -1;
    }
}

int Core::initTrustLinesManager() {

    try{
        mTrustLinesManager = new TrustLinesManager(&mLog);
        mLog.logSuccess("Core", "Trust lines manager is successfully initialised");
        return 0;

    }catch(const std::exception &e) {
        mLog.logException("Core", e);
        return -1;
    }
}

int Core::initTransactionsManager() {

    try {
        mTransactionsManager = new TransactionsManager(
            mNodeUUID,
            mIOService,
            mTrustLinesManager,
            mResultsInterface,
            &mLog
        );
        mLog.logSuccess("Core", "Transactions handler is successfully initialised");
        return 0;

    } catch (const std::exception &e) {
        mLog.logException("Core", e);
        return -1;
    }
}

int Core::initCommandsInterface() {

    try {
        mCommandsInterface = new CommandsInterface(
            mIOService,
            mTransactionsManager,
            &mLog
        );
        mLog.logSuccess("Core", "Commands interface is successfully initialised");
        return 0;

    } catch (const std::exception &e) {
        mLog.logException("Core", e);
        return -1;
    }
}

void Core::connectCommunicatorSignals() {

    //communicator's signal to transactions manager slot
    mCommunicator->messageReceivedSignal.connect(
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
}
void Core::connectDelayedTasksSignals(){
    mCyclesDelayedTasks->mSixNodesCycleSignal.connect(
            boost::bind(
                    &Core::onDelayedTaskCycleSixNodesSlot,
                    this
            )
    );
    mCyclesDelayedTasks->mFiveNodesCycleSignal.connect(
            boost::bind(
                    &Core::onDelayedTaskCycleFiveNodesSlot,
                    this
            )
    );
}
void Core::connectSignalsToSlots() {

    connectCommunicatorSignals();
    connectTrustLinesManagerSignals();
    connectDelayedTasksSignals();
}

void Core::onMessageReceivedSlot(
    Message::Shared message) {

    try {
        mTransactionsManager->processMessage(message);

    } catch(exception &e) {
        mLog.logException("Core", e);
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
        mLog.logException("Core", e);
    }
}

void Core::onTrustLineCreatedSlot(
    const NodeUUID &contractorUUID,
    const TrustLineUUID &trustLineUUID) {

    try{
        mTransactionsManager->launchRoutingTablePropagationTransaction(
            contractorUUID,
            trustLineUUID
        );

    } catch (exception &e) {
        mLog.logException("Core", e);
    }
}

void Core::cleanupMemory() {

    if (mSettings != nullptr) {
        delete mSettings;
    }

    if (mCommunicator != nullptr) {
        delete mCommunicator;
    }

    if (mResultsInterface != nullptr) {
        delete mResultsInterface;
    }

    if (mTrustLinesManager != nullptr) {
        delete mTrustLinesManager;
    }

    if (mTransactionsManager != nullptr) {
        delete mTransactionsManager;
    }

    if (mCommandsInterface != nullptr) {
        delete mCommandsInterface;
    }
}

void Core::zeroPointers() {

    mSettings = nullptr;
    mCommunicator = nullptr;
    mCommandsInterface = nullptr;
    mResultsInterface = nullptr;
    mTrustLinesManager = nullptr;
    mTransactionsManager = nullptr;
    mCyclesDelayedTasks = nullptr;
}

//void Core::initTimers() {
//
//}

int Core::initDelayedTasks() {
    try{
        mCyclesDelayedTasks = new CyclesDelayedTasks(
               mIOService
        );
    mLog.logSuccess("Core", "DelayedTasks is successfully initialised");
    return 0;
    } catch (const std::exception &e) {
        mLog.logException("Core", e);
        return -1;
    }
}

void Core::onDelayedTaskCycleSixNodesSlot() {
//    mTransactionsManager->launchGetTopologyAndBalancesTransaction();
}

void Core::onDelayedTaskCycleFiveNodesSlot() {
//    mTransactionsManager->launchGetTopologyAndBalancesTransaction();
}

void Core::JustToTestSomething() {
//    mTrustLinesManager->getFirstLevelNodesForCycles();
//    auto firstLevelNodes = mTrustLinesManager->getFirstLevelNodesForCycles();
    vector<NodeUUID> path;
    path.push_back(mNodeUUID);
    TrustLineBalance bal = 70;
//    for(const auto &value: firstLevelNodes){

//
    auto message = Message::Shared(new InBetweenNodeTopologyMessage(
            bal,
            2,
            path));
    mTransactionsManager->launchGetTopologyAndBalancesTransaction(static_pointer_cast<InBetweenNodeTopologyMessage>(
            message
    )
    );
}
