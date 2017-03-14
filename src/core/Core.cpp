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
        mLog.logFatal("Core", "Can't read UUID of the node from the settings.");
        return -1;
    }

    initCode = initOperationsHistoryStorage();
    if (initCode != 0)
        return initCode;

    initCode = initCommunicator(conf);
    if (initCode != 0)
        return initCode;

    initCode = initResultsInterface();
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

int Core::initOperationsHistoryStorage() {

    try{
        mOperationsHistoryStorage = new history::OperationsHistoryStorage(
            "io",
            "operations_history_storage.dat");

        mLog.logSuccess("Core", "Operations history storage is successfully initialised");
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
        mLog.logSuccess("Core", "Network communicator is successfully initialised");
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

int Core::initMaxFlowCalculationTrustLineManager() {

    try{
        mMaxFlowCalculationTrustLimeManager = new MaxFlowCalculationTrustLineManager(&mLog);
        mLog.logSuccess("Core", "Max flow calculation Trust lines manager is successfully initialised");
        return 0;

    }catch(const std::exception &e) {
        mLog.logException("Core", e);
        return -1;
    }
}

int Core::initMaxFlowCalculationCacheManager() {

    try {
        mMaxFlowCalculationCacheManager = new MaxFlowCalculationCacheManager(&mLog);
        mLog.logSuccess("Core", "Max flow calculation Cache manager is successfully initialised");
        return 0;
    } catch (const std::exception &e) {
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
            mMaxFlowCalculationTrustLimeManager,
            mMaxFlowCalculationCacheManager,
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

int Core::initDelayedTasks() {
    try{
        mCyclesDelayedTasks = new CyclesDelayedTasks(
                mIOService);
        mMaxFlowCalculationCacheUpdateDelayedTask = new MaxFlowCalculationCacheUpdateDelayedTask(
                mIOService);
        mLog.logSuccess("Core", "DelayedTasks is successfully initialised");
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

    mTrustLinesManager->trustLineStateModifiedSignal.connect(
        boost::bind(
            &Core::onTrustLineStateModifiedSlot,
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
    mMaxFlowCalculationCacheUpdateDelayedTask->mMaxFlowCalculationCacheUpdateSignal.connect(
        boost::bind(
            &Core::onDelayedTaskMaxFlowCalculationCacheUpdateSlot,
            this));
}

void Core::connectSignalsToSlots() {

    connectCommunicatorSignals();
    connectTrustLinesManagerSignals();
    connectDelayedTasksSignals();
}

void Core::onMessageReceivedSlot(
    Message::Shared message) {

//    try {
        mTransactionsManager->processMessage(message);

//    } catch(exception &e) {
//        mLog.logException("Core", e);
//    }
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
    const TrustLineDirection direction) {

#ifdef ROUTING_TABLES_PROPAGATION_DISABLED
    return;
#endif


    try {
        mTransactionsManager->launchFromInitiatorToContractorRoutingTablePropagationTransaction(
            contractorUUID,
            direction
        );

    } catch (exception &e) {
        mLog.logException("Core", e);
    }
}

void Core::onTrustLineStateModifiedSlot(
    const NodeUUID &contractorUUID,
    const TrustLineDirection direction) {

    try {
        mTransactionsManager->launchRoutingTablesUpdatingTransactionsFactory(
            contractorUUID,
            direction);

    } catch (exception &e) {
        mLog.logException("Core", e);
    }

}

void Core::onDelayedTaskCycleSixNodesSlot() {
//    mTransactionsManager->launchGetTopologyAndBalancesTransaction();
}

void Core::onDelayedTaskCycleFiveNodesSlot() {
//    mTransactionsManager->launchGetTopologyAndBalancesTransaction();
}

void Core::cleanupMemory() {

    if (mSettings != nullptr) {
        delete mSettings;
    }

    if (mOperationsHistoryStorage != nullptr) {
        delete mOperationsHistoryStorage;
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

    if (mMaxFlowCalculationTrustLimeManager != nullptr) {
        delete mMaxFlowCalculationTrustLimeManager;
    }

    if (mMaxFlowCalculationCacheManager != nullptr) {
        delete mMaxFlowCalculationCacheManager;
    }

    if (mMaxFlowCalculationCacheUpdateDelayedTask != nullptr) {
        delete mMaxFlowCalculationCacheUpdateDelayedTask;
    }
}

void Core::zeroPointers() {

    mSettings = nullptr;
    mOperationsHistoryStorage = nullptr;
    mCommunicator = nullptr;
    mCommandsInterface = nullptr;
    mResultsInterface = nullptr;
    mTrustLinesManager = nullptr;
    mTransactionsManager = nullptr;
    mCyclesDelayedTasks = nullptr;
    mMaxFlowCalculationTrustLimeManager = nullptr;
    mMaxFlowCalculationCacheManager = nullptr;
    mMaxFlowCalculationCacheUpdateDelayedTask = nullptr;
}

//void Core::initTimers() {
//
//}

void Core::JustToTestSomething() {
//    mTrustLinesManager->getFirstLevelNodesForCycles();
//    auto firstLevelNodes = mTrustLinesManager->getFirstLevelNodesForCycles();
//    TrustLineBalance bal = 70;
//    TrustLineBalance max_flow = 30;
//    vector<NodeUUID> path;
//    vector<pair<NodeUUID, TrustLineBalance>> boundaryNodes;
//    boundaryNodes.push_back(make_pair(mNodeUUID, bal ));
//    path.push_back(mNodeUUID);
////    for(const auto &value: firstLevelNodes){
//
////
//    auto message = Message::Shared(new BoundaryNodeTopolodyMessage(
//            max_flow,
//            2,
//            path,
//            boundaryNodes
//    ));
//    auto buffer = message->serializeToBytes();
//    auto new_message = new BoundaryNodeTopolodyMessage(buffer.first);
//    cout << "lets see what we have " << endl;
    //    mTransactionsManager->launchGetTopologyAndBalancesTransaction(static_pointer_cast<BoundaryNodeTopologyMessage>(
//            message
//    )
//    );
}

void Core::onDelayedTaskMaxFlowCalculationCacheUpdateSlot() {
    mTransactionsManager->launchMaxFlowCalculationCacheUpdateTransaction();
}
