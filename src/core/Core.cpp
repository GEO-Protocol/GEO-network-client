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
        writePIDFile();

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
        mTrustLinesManager = new TrustLinesManager(
            mStorageHandler,
            &mLog);
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

int Core::initResourcesManager() {

    try {
        mResourcesManager = new ResourcesManager();
        mLog.logSuccess("Core", "Resources manager is successfully initialized");
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
            mResourcesManager,
            mMaxFlowCalculationTrustLimeManager,
            mMaxFlowCalculationCacheManager,
            mResultsInterface,
            mStorageHandler,
            mPathsManager,
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
                mIOService,
                mMaxFlowCalculationCacheManager,
                mMaxFlowCalculationTrustLimeManager,
                &mLog);
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
            &mLog
        );
        mLog.logSuccess("Core", "Commands interface is successfully initialised");
        return 0;

    } catch (const std::exception &e) {
        mLog.logException("Core", e);
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
        mStorageHandler = new StorageHandler(
            "io",
            "storageDB",
            &mLog);
        mLog.logSuccess("Core", "Storage handler is successfully initialised");
        return 0;
    } catch (const std::exception &e) {
        mLog.logException("Core", e);
        return -1;
    }
}

int Core::initPathsManager() {

    try {
        mPathsManager = new PathsManager(
            mNodeUUID,
            mTrustLinesManager,
            mStorageHandler,
            &mLog);
        mLog.logSuccess("Core", "Paths Manager is successfully initialised");
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
//    mCyclesDelayedTasks->mSixNodesCycleSignal.connect(
//            boost::bind(
//                    &Core::onDelayedTaskCycleSixNodesSlot,
//                    this
//            )
//    );
//    mCyclesDelayedTasks->mFiveNodesCycleSignal.connect(
//            boost::bind(
//                    &Core::onDelayedTaskCycleFiveNodesSlot,
//                    this
//            )
//    );
//    #ifdef TESTS
//    mCyclesDelayedTasks->mThreeNodesCycleSignal.connect(
//            boost::bind(
//                    &Core::onDelayedTaskCycleThreeNodesSlot,
//                    this
//            )
//    );
//    mCyclesDelayedTasks->mFourNodesCycleSignal.connect(
//            boost::bind(
//                    &Core::onDelayedTaskCycleFourNodesSlot,
//                    this
//            )
//    );
//    #endif
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
void Core::connectSignalsToSlots() {

    connectCommandsInterfaceSignals();
    connectCommunicatorSignals();
    connectTrustLinesManagerSignals();
    connectDelayedTasksSignals();
    connectResourcesManagerSignals();
}

void Core::onCommandReceivedSlot (
    BaseUserCommand::Shared command)
{
    try {
        mTransactionsManager->processCommand(command);

    } catch(exception &e) {
        mLog.logException("Core", e);
    }
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
    const TrustLineDirection direction) {

}

void Core::onTrustLineStateModifiedSlot(
    const NodeUUID &contractorUUID,
    const TrustLineDirection direction) {

}

void Core::onDelayedTaskCycleSixNodesSlot() {
    mTransactionsManager->launchSixNodesCyclesInitTransaction();
}

void Core::onDelayedTaskCycleFiveNodesSlot() {
    mTransactionsManager->launchFiveNodesCyclesInitTransaction();
}

void Core::onDelayedTaskCycleFourNodesSlot() {
//    test_FourNodesTransaction();
}

void Core::onDelayedTaskCycleThreeNodesSlot() {
//    test_ThreeNodesTransaction();
}

void Core::onPathsResourceRequestedSlot(
    const TransactionUUID &transactionUUID,
    const NodeUUID &destinationNodeUUID) {
    try {
        mTransactionsManager->launchPathsResourcesCollectTransaction(
            transactionUUID,
            destinationNodeUUID);

    } catch (exception &e) {
        mLog.logException("Core", e);
    }

}

void Core::onResourceCollectedSlot(
    BaseResource::Shared resource) {

    try {
        mTransactionsManager->attachResourceToTransaction(
            resource);

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

    if (mResourcesManager != nullptr) {
        delete mResourcesManager;
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

/*    if (mStorageHandler != nullptr) {
        delete mStorageHandler;
    }*/

    if (mPathsManager != nullptr) {
        delete mPathsManager;
    }
}

void Core::zeroPointers() {

    mSettings = nullptr;
    mCommunicator = nullptr;
    mCommandsInterface = nullptr;
    mResultsInterface = nullptr;
    mTrustLinesManager = nullptr;
    mResourcesManager = nullptr;
    mTransactionsManager = nullptr;
    mCyclesDelayedTasks = nullptr;
    mMaxFlowCalculationTrustLimeManager = nullptr;
    mMaxFlowCalculationCacheManager = nullptr;
    mMaxFlowCalculationCacheUpdateDelayedTask = nullptr;
}

//}

//void Core::JustToTestSomething() {
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
//}
//
//void Core::onDelayedTaskMaxFlowCalculationCacheUpdateSlot() {
//    mTransactionsManager->launchMaxFlowCalculationCacheUpdateTransaction();
//}

void Core::writePIDFile()
{
    try {
        std::ofstream pidFile("process.pid");
        pidFile << ::getpid() << std::endl;
        pidFile.close();

    } catch (std::exception &e) {
        auto errors = mLog.error("Core");
        errors << "Can't write/update pid file. Error message is: " << e.what();
    }
}

void Core::checkSomething() {
    auto debtorsNeighborsUUIDs = mTrustLinesManager->firstLevelNeighborsWithPositiveBalance();
    stringstream ss;
    copy(debtorsNeighborsUUIDs.begin(), debtorsNeighborsUUIDs.end(), ostream_iterator<NodeUUID>(ss, "\n"));
    cout << "Nodes With positive balance: \n" << ss.str() << endl;
    auto creditorsNeighborsUUIDs = mTrustLinesManager->firstLevelNeighborsWithNegativeBalance();
    stringstream ss1;
    copy(creditorsNeighborsUUIDs.begin(), creditorsNeighborsUUIDs.end(), ostream_iterator<NodeUUID>(ss1, "\n"));
    cout << "Nodes With negative balance: \n" << ss1.str() << endl;
}

void Core::printRTs() {
    NodeUUID *some_node = new NodeUUID("65b84dc1-31f8-45ce-8196-8efcc7648777");
    NodeUUID *dest_node = new NodeUUID("5062d6a9-e06b-4bcc-938c-6d9bd082f0eb");
    mStorageHandler->routingTablesHandler()->setRecordToRT2(*some_node, *dest_node);

    cout  << "printRTs\tRT1 size: " << mTrustLinesManager->trustLines().size();
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

void Core::test_ThreeNodesTransaction() {
    cout << "Nodes With Positive Balance" << endl;
    const auto kNeighborsUUIDs = mTrustLinesManager->getFirstLevelNodesForCycles(0);
    stringstream ss;
    copy(kNeighborsUUIDs.begin(), kNeighborsUUIDs.end(), ostream_iterator<NodeUUID>(ss, "\n"));
    cout << "test_ThreeNodesTransaction::Nodes With positive balance: \n" << ss.str() << endl;
    for(const auto &kNodeUUID: kNeighborsUUIDs) {
            mTransactionsManager->launchThreeNodesCyclesInitTransaction(kNodeUUID);
    }
}

void Core::test_FourNodesTransaction() {
    cout << "Nodes With Positive Balance" << endl;
    auto debtorsNeighborsUUIDs = mTrustLinesManager->firstLevelNeighborsWithPositiveBalance();
    stringstream ss;
    copy(debtorsNeighborsUUIDs.begin(), debtorsNeighborsUUIDs.end(), ostream_iterator<NodeUUID>(ss, "\n"));
    cout << "test_FourNodesTransaction::Nodes With positive balance: \n" << ss.str() << endl;
    auto creditorsNeighborsUUIDs = mTrustLinesManager->firstLevelNeighborsWithNegativeBalance();
    stringstream ss1;
    copy(creditorsNeighborsUUIDs.begin(), creditorsNeighborsUUIDs.end(), ostream_iterator<NodeUUID>(ss1, "\n"));
    cout << "test_FourNodesTransaction::Nodes With negative balance: \n" << ss1.str() << endl;
    for(const auto &kCreditorNodeUUID: creditorsNeighborsUUIDs) {
        for (const auto &kDebtorNodeUUID: debtorsNeighborsUUIDs) {
            cout << "_______________________________" << endl;
            cout << "Debtor UUID:  " << kDebtorNodeUUID << endl;
            cout << "Credior UUID:   " << kCreditorNodeUUID << endl;
            mTransactionsManager->launchFourNodesCyclesInitTransaction(kDebtorNodeUUID, kCreditorNodeUUID);
        }
    }
}
