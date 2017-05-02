#include "PathsManager.h"

PathsManager::PathsManager(
    const NodeUUID &nodeUUID,
    TrustLinesManager *trustLinesManager,
    StorageHandler *storageHandler,
    Logger *logger):

    mNodeUUID(nodeUUID),
    mTrustLinesManager(trustLinesManager),
    mStorageHandler(storageHandler),
    mLog(logger),
    mPathCollection(nullptr)
{
    // TODO remove from here
    //testStorageHandler();
    //fillRoutingTables();
    //fillBigRoutingTables();
    //testTrustLineHandler();
    //testPaymentStateOperationsHandler();
    //testTransactionHandler();
    //testTime();
    //testMultiConnection();
    //printRTs();
    //testDeletingRT();
    //fillCycleTablesTestCase0();
    //fillCycleTablesTestCase1();
    //fillCycleTablesTestCase2();
    //fillCycleTablesTestCase3();
}

void PathsManager::findDirectPath()
{
    for (auto const &nodeUUID : mTrustLinesManager->rt1()) {
        if (nodeUUID == mContractorUUID) {
            Path path = Path(
                mNodeUUID,
                mContractorUUID);
            mPathCollection->add(path);
#ifdef GETTING_PATHS_DEBUG_LOG
            info() << "found direct path";
#endif
            return;
        }
    }
}

void PathsManager::findPathsOnSecondLevel()
{
#ifdef GETTING_PATHS_DEBUG_LOG
    DateTime startTime = utc_now();
#endif
    auto ioTransaction = mStorageHandler->beginTransaction();
    for (auto const &nodeUUID : ioTransaction->routingTablesHandler()->subRoutesSecondLevel(mContractorUUID)) {
        vector<NodeUUID> intermediateNodes;
        intermediateNodes.push_back(nodeUUID);
        Path path(
            mNodeUUID,
            mContractorUUID,
            intermediateNodes);
        mPathCollection->add(path);
#ifdef GETTING_PATHS_DEBUG_LOG
        info() << "found path on second level";
#endif
    }
#ifdef GETTING_PATHS_DEBUG_LOG
    Duration methodTime = utc_now() - startTime;
    info() << "findPathsOnSecondLevel method time: " << methodTime;
#endif
}

void PathsManager::findPathsOnSecondLevelWithoutRoutingTables(
    vector<NodeUUID> &contractorRT1)
{
#ifdef GETTING_PATHS_DEBUG_LOG
    DateTime startTime = utc_now();
#endif
    for (auto const &nodeUUID1 : mTrustLinesManager->rt1()) {
        for (auto const &nodeUUID2 : contractorRT1) {
            if (nodeUUID1 == nodeUUID2) {
                vector<NodeUUID> intermediateNodes;
                intermediateNodes.push_back(nodeUUID1);
                Path path(
                    mNodeUUID,
                    mContractorUUID,
                    intermediateNodes);
                mPathCollection->add(path);
#ifdef GETTING_PATHS_DEBUG_LOG
                info() << "found path on second level";
#endif
            }
        }
    }
#ifdef GETTING_PATHS_DEBUG_LOG
    Duration methodTime = utc_now() - startTime;
    info() << "findPathsOnSecondLevel method time: " << methodTime;
#endif
}

void PathsManager::findPathsOnThirdLevel()
{
#ifdef GETTING_PATHS_DEBUG_LOG
    DateTime startTime = utc_now();
#endif
    auto ioTransaction = mStorageHandler->beginTransaction();
    for (auto const &nodeUUIDAndNodeUUID : ioTransaction->routingTablesHandler()->subRoutesThirdLevelContractor(
            mContractorUUID,
            mNodeUUID)) {
        vector<NodeUUID> intermediateNodes;
        intermediateNodes.push_back(nodeUUIDAndNodeUUID.first);
        intermediateNodes.push_back(nodeUUIDAndNodeUUID.second);
        Path path(
            mNodeUUID,
            mContractorUUID,
            intermediateNodes);
        mPathCollection->add(path);
#ifdef GETTING_PATHS_DEBUG_LOG
        info() << "found path on third level";
#endif
    }
#ifdef GETTING_PATHS_DEBUG_LOG
    Duration methodTime = utc_now() - startTime;
    info() << "findPathsOnThirdLevel method time: " << methodTime;
#endif
}

void PathsManager::findPathsOnForthLevel(
    vector<NodeUUID> &contractorRT1)
{
#ifdef GETTING_PATHS_DEBUG_LOG
    DateTime startTime =  utc_now();
#endif
    auto ioTransaction = mStorageHandler->beginTransaction();
    for (auto const &nodeUUID : contractorRT1) {
        if (nodeUUID == mNodeUUID) {
            continue;
        }
        for (auto const &nodeUUIDAndNodeUUID : ioTransaction->routingTablesHandler()->subRoutesThirdLevelWithForbiddenNodes(
                nodeUUID,
                mNodeUUID,
                mContractorUUID)) {
            vector<NodeUUID> intermediateNodes;
            intermediateNodes.push_back(nodeUUIDAndNodeUUID.first);
            intermediateNodes.push_back(nodeUUIDAndNodeUUID.second);
            intermediateNodes.push_back(nodeUUID);
            Path path(
                mNodeUUID,
                mContractorUUID,
                intermediateNodes);
            mPathCollection->add(path);
#ifdef GETTING_PATHS_DEBUG_LOG
            info() << "found path on forth level";
#endif
        }
    }
#ifdef GETTING_PATHS_DEBUG_LOG
    Duration methodTime = utc_now() - startTime;
    info() << "findPathsOnForthLevel method time: " << methodTime;
#endif
}

void PathsManager::findPathsOnFifthLevel(
    unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> &contractorRT2)
{
#ifdef GETTING_PATHS_DEBUG_LOG
    DateTime startTime = utc_now();
#endif
    auto ioTransaction = mStorageHandler->beginTransaction();
    for (auto const &itRT2 : contractorRT2) {
        // TODO (mc) : need or not second condition (itRT2.first == contractorUUID)
        if (itRT2.first == mNodeUUID || itRT2.first == mContractorUUID) {
            continue;
        }
        for (auto const &nodeUUIDAndNodeUUID : ioTransaction->routingTablesHandler()->subRoutesThirdLevelWithForbiddenNodes(
                itRT2.first,
                mNodeUUID,
                mContractorUUID)) {
            vector<NodeUUID> intermediateNodes;
            intermediateNodes.push_back(nodeUUIDAndNodeUUID.first);
            intermediateNodes.push_back(nodeUUIDAndNodeUUID.second);
            intermediateNodes.push_back(itRT2.first);
            for (auto const &nodeUUID : itRT2.second) {
                if(std::find(intermediateNodes.begin(), intermediateNodes.end(), nodeUUID) != intermediateNodes.end() ||
                        nodeUUID == mNodeUUID || nodeUUID == mContractorUUID) {
                    continue;
                }
                intermediateNodes.push_back(nodeUUID);
                Path path(
                    mNodeUUID,
                    mContractorUUID,
                    intermediateNodes);
                mPathCollection->add(path);
                intermediateNodes.pop_back();
#ifdef GETTING_PATHS_DEBUG_LOG
                info() << "found path on fifth level";
#endif
            }
        }
    }
#ifdef GETTING_PATHS_DEBUG_LOG
    Duration methodTime = utc_now() - startTime;
    info() << "findPathsOnFifthLevel method time: " << methodTime;
#endif
}

void PathsManager::findPathsOnSixthLevel(
    unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> &contractorRT3,
    unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> &contractorRT2)
{
#ifdef GETTING_PATHS_DEBUG_LOG
    DateTime startTime = utc_now();
#endif
    auto ioTransaction = mStorageHandler->beginTransaction();
    for (auto const &itRT3 : contractorRT3) {
        // TODO (mc) : need or not second condition (itRT3.first == contractorUUID)
        if (itRT3.first == mNodeUUID || itRT3.first == mContractorUUID) {
            continue;
        }
        for (auto const &nodeUUIDAndNodeUUID : ioTransaction->routingTablesHandler()->subRoutesThirdLevelWithForbiddenNodes(
                itRT3.first,
                mNodeUUID,
                mContractorUUID)) {
            vector<NodeUUID> intermediateNodes;
            intermediateNodes.push_back(nodeUUIDAndNodeUUID.first);
            intermediateNodes.push_back(nodeUUIDAndNodeUUID.second);
            intermediateNodes.push_back(itRT3.first);
            for (auto const &nodeUUID : itRT3.second) {
                if(std::find(intermediateNodes.begin(), intermediateNodes.end(), nodeUUID) != intermediateNodes.end() ||
                   nodeUUID == mNodeUUID || nodeUUID == mContractorUUID) {
                    continue;
                }
                intermediateNodes.push_back(nodeUUID);
                for (auto &contactorIntermediateNode : intermediateNodesOnContractorFirstLevel(
                        nodeUUID,
                        intermediateNodes,
                        contractorRT2)) {
                    intermediateNodes.push_back(contactorIntermediateNode);
                    Path path(
                        mNodeUUID,
                        mContractorUUID,
                        intermediateNodes);
                    mPathCollection->add(path);
                    intermediateNodes.pop_back();
#ifdef GETTING_PATHS_DEBUG_LOG
                    info() << "found path on sixth level";
#endif
                }
                intermediateNodes.pop_back();
            }
        }
    }
#ifdef GETTING_PATHS_DEBUG_LOG
    Duration methodTime = utc_now() - startTime;
    info() << "findPathsOnSixthLevel method time: " << methodTime;
#endif
}

vector<NodeUUID> PathsManager::intermediateNodesOnContractorFirstLevel(
    const NodeUUID &thirdLevelSourceNode,
    vector<NodeUUID> &intermediateNodes,
    unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> &contractorRT2) const
{
    auto nodeUUIDAndVect = contractorRT2.find(thirdLevelSourceNode);
    if (nodeUUIDAndVect == contractorRT2.end()) {
        return {};
    } else {
        vector<NodeUUID> result;
        for (auto const &nodeUUID : nodeUUIDAndVect->second) {
            if(std::find(intermediateNodes.begin(), intermediateNodes.end(), nodeUUID) != intermediateNodes.end() ||
                    nodeUUID == mNodeUUID || nodeUUID == mContractorUUID) {
                continue;
            }
            result.push_back(nodeUUID);
        }
        return result;
    }
}

// test
void PathsManager::findPathsOnSecondLevelTest()
{
    DateTime startTime = utc_now();
    auto ioTransaction = mStorageHandler->beginTransaction();
    for (auto const &nodeUUID : ioTransaction->routingTablesHandler()->subRoutesSecondLevel(mContractorUUID)) {
        Path path(
            mNodeUUID,
            mContractorUUID,
            {nodeUUID});
        if (isPathValid(path)) {
            mPathCollection->add(path);
            //info() << "found path on second level";
        }
    }
    /*Duration methodTime = utc_now() - startTime;
    info() << "findPathsOnSecondLevel test method time: " << methodTime;*/
}

void PathsManager::findPathsOnThirdLevelTest()
{
    DateTime startTime = utc_now();
    auto ioTransaction = mStorageHandler->beginTransaction();
    for (auto const &nodeUUIDAndNodeUUID : ioTransaction->routingTablesHandler()->subRoutesThirdLevel(
            mContractorUUID)) {
        if (nodeUUIDAndNodeUUID.first == mContractorUUID || nodeUUIDAndNodeUUID.second == mNodeUUID) {
            continue;
        }
        Path path(
            mNodeUUID,
            mContractorUUID,
            {nodeUUIDAndNodeUUID.first,
             nodeUUIDAndNodeUUID.second});
        if (isPathValid(path)) {
            mPathCollection->add(path);
            //info() << "found path on third level";
        }
    }
    /*Duration methodTime = utc_now() - startTime;
    info() << "findPathsOnThirdLevel test method time: " << methodTime;*/
}

void PathsManager::findPathsOnForthLevelTest(
    vector<NodeUUID> &contractorRT1)
{
    DateTime startTime = utc_now();
    auto ioTransaction = mStorageHandler->beginTransaction();
    for (auto const &nodeUUID : contractorRT1) {
        if (nodeUUID == mNodeUUID) {
            continue;
        }
        for (auto const &nodeUUIDAndNodeUUID : ioTransaction->routingTablesHandler()->subRoutesThirdLevel(
                nodeUUID)) {
            Path path(
                mNodeUUID,
                mContractorUUID,
                {nodeUUIDAndNodeUUID.first,
                 nodeUUIDAndNodeUUID.second,
                 nodeUUID});
            if (isPathValid(path)) {
                mPathCollection->add(path);
                //info() << "found path on forth level";
            }
        }
    }
    /*Duration methodTime = utc_now() - startTime;
    info() << "findPathsOnForthLevel test method time: " << methodTime;*/
}

void PathsManager::findPathsOnFifthLevelTest(
    unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> &contractorRT2)
{
    DateTime startTime = utc_now();
    auto ioTransaction = mStorageHandler->beginTransaction();
    for (auto const &itRT2 : contractorRT2) {
        // TODO (mc) : need or not second condition (itRT2.first == contractorUUID)
        if (itRT2.first == mNodeUUID || itRT2.first == mContractorUUID) {
            continue;
        }
        for (auto const &nodeUUIDAndNodeUUID : ioTransaction->routingTablesHandler()->subRoutesThirdLevel(
                itRT2.first)) {
            for (auto const &nodeUUID : itRT2.second) {
                vector<NodeUUID> intermediateNodes;
                intermediateNodes.push_back(nodeUUIDAndNodeUUID.first);
                intermediateNodes.push_back(nodeUUIDAndNodeUUID.second);
                intermediateNodes.push_back(itRT2.first);
                intermediateNodes.push_back(nodeUUID);
                Path path(
                    mNodeUUID,
                    mContractorUUID,
                    intermediateNodes);
                if (isPathValid(path)) {
                    mPathCollection->add(path);
                    //info() << "found path on fifth level test";
                }
            }
        }
    }
    Duration methodTime = utc_now() - startTime;
    info() << "findPathsOnFifthLevel test method time: " << methodTime;
}

void PathsManager::findPathsOnSixthLevelTest(
    unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> &contractorRT3,
    unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> &contractorRT2)
{
    DateTime startTime = utc_now();
    auto ioTransaction = mStorageHandler->beginTransaction();
    for (auto const &itRT3 : contractorRT3) {
        // TODO (mc) : need or not second condition (itRT3.first == contractorUUID)
        if (itRT3.first == mNodeUUID || itRT3.first == mContractorUUID) {
            continue;
        }
        for (auto const &nodeUUIDAndNodeUUID : ioTransaction->routingTablesHandler()->subRoutesThirdLevel(
                itRT3.first)) {

            for (auto const &nodeUUID : itRT3.second) {
                for (auto &contactorIntermediateNode : intermediateNodesOnContractorFirstLevelTest(
                        nodeUUID,
                        contractorRT2)) {
                    vector<NodeUUID> intermediateNodes;
                    intermediateNodes.push_back(nodeUUIDAndNodeUUID.first);
                    intermediateNodes.push_back(nodeUUIDAndNodeUUID.second);
                    intermediateNodes.push_back(itRT3.first);
                    intermediateNodes.push_back(nodeUUID);
                    intermediateNodes.push_back(contactorIntermediateNode);
                    Path path(
                        mNodeUUID,
                        mContractorUUID,
                        intermediateNodes);
                    if (isPathValid(path)) {
                        mPathCollection->add(path);
                        //info() << "found path on sixth level test";
                    }
                }
            }
        }
    }
    /*Duration methodTime = utc_now() - startTime;
    info() << "findPathsOnSixthLevel test method time: " << methodTime;*/
}

vector<NodeUUID> PathsManager::intermediateNodesOnContractorFirstLevelTest(
    const NodeUUID &thirdLevelSourceNode,
    unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> &contractorRT2) const
{
    auto nodeUUIDAndVect = contractorRT2.find(thirdLevelSourceNode);
    if (nodeUUIDAndVect == contractorRT2.end()) {
        return {};
    } else {
        return nodeUUIDAndVect->second;
    }
}

bool PathsManager::isPathValid(const Path &path)
{
    auto itGlobal = path.nodes.begin();
    while (itGlobal != path.nodes.end() - 1) {
        auto itLocal = itGlobal + 1;
        while (itLocal != path.nodes.end()) {
            if (*itGlobal == *itLocal) {
                return false;
            }
            itLocal++;
        }
        itGlobal++;
    }
    return true;
}
// test end

void PathsManager::findPaths(
    const NodeUUID &contractorUUID,
    vector<NodeUUID> &contractorRT1,
    unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> &contractorRT2,
    unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> &contractorRT3)
{
    mContractorUUID = contractorUUID;
#ifdef GETTING_PATHS_DEBUG_LOG
    info() << "start finding paths to " << contractorUUID;
    DateTime startTime = utc_now();
#endif
    mPathCollection = make_shared<PathsCollection>(
        mNodeUUID,
        mContractorUUID);
    findDirectPath();
    findPathsOnSecondLevel();
    //findPathsOnSecondLevelWithoutRoutingTables(contractorRT1);
    findPathsOnThirdLevel();
    findPathsOnForthLevel(
        contractorRT1);
    findPathsOnFifthLevel(
        contractorRT2);
    findPathsOnSixthLevel(
        contractorRT3,
        contractorRT2);
#ifdef GETTING_PATHS_DEBUG_LOG
    Duration methodTime = utc_now() - startTime;
    info() << "PathsManager::findPath\tmethod time: " << methodTime;
    info() << "total paths count: " << mPathCollection->count();
    while (mPathCollection->hasNextPath()) {
        //info() << mPathCollection->nextPath()->toString();
        if (!isPathValid(*mPathCollection->nextPath().get())) {
            info() << "wrong path!!! ";
        }
    }
#endif
}

void PathsManager::findPathsOnSelfArea(
    const NodeUUID &contractorUUID)
{
    mContractorUUID = contractorUUID;
#ifdef GETTING_PATHS_DEBUG_LOG
    info() << "start finding paths on self area to " << contractorUUID;
    DateTime startTime = utc_now();
#endif
    mPathCollection = make_shared<PathsCollection>(
        mNodeUUID,
        mContractorUUID);
    findDirectPath();
    findPathsOnSecondLevel();
    findPathsOnThirdLevel();
#ifdef GETTING_PATHS_DEBUG_LOG
    Duration methodTime = utc_now() - startTime;
    info() << "PathsManager::findPathsOnSelfArea\tmethod time: " << methodTime;
    info() << "total paths on self area count: " << mPathCollection->count();
    /*while (mPathCollection->hasNextPath()) {
        info() << mPathCollection->nextPath()->toString();
    }*/
#endif
}

void PathsManager::findPathsTest(
    const NodeUUID &contractorUUID,
    vector<NodeUUID> &contractorRT1,
    unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> &contractorRT2,
    unordered_map<NodeUUID, vector<NodeUUID>, boost::hash<boost::uuids::uuid>> &contractorRT3)
{
    mContractorUUID = contractorUUID;
    info() << "start finding test paths to " << mContractorUUID;
    DateTime startTime = utc_now();
    mPathCollection = make_shared<PathsCollection>(
        mNodeUUID,
        mContractorUUID);
    findDirectPath();
    findPathsOnSecondLevelTest();
    findPathsOnThirdLevelTest();
    findPathsOnForthLevelTest(
        contractorRT1);
    findPathsOnFifthLevelTest(
        contractorRT2);
    findPathsOnSixthLevelTest(
        contractorRT3,
        contractorRT2);
    Duration methodTime = utc_now() - startTime;
    info() << "PathsManager::findPathTest\tmethod time: " << methodTime;
    info() << "total paths test count: " << mPathCollection->count();
    /*while (mPathCollection->hasNextPath()) {
        info() << mPathCollection->nextPath()->toString();
    }*/
}

PathsCollection::Shared PathsManager::pathCollection() const
{
    return mPathCollection;
}

void PathsManager::clearPathsCollection()
{
    mPathCollection = nullptr;
}

void PathsManager::fillRoutingTables()
{
    NodeUUID* nodeUUID90Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff90");
    NodeUUID* nodeUUID91Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff91");
    NodeUUID* nodeUUID92Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff92");
    NodeUUID* nodeUUID93Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff93");
    NodeUUID* nodeUUID94Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff94");
    NodeUUID* nodeUUID95Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff95");
    NodeUUID* nodeUUID96Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff96");
    NodeUUID* nodeUUID97Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff97");
    NodeUUID* nodeUUID98Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff98");

    auto ioTransaction = mStorageHandler->beginTransaction();
    if (!mNodeUUID.stringUUID().compare("13e5cf8c-5834-4e52-b65b-f9281dd1ff90")) {
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID92Ptr, *nodeUUID94Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID92Ptr, *nodeUUID90Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID94Ptr, *nodeUUID92Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID94Ptr, *nodeUUID96Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID94Ptr, *nodeUUID97Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID94Ptr, *nodeUUID98Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID94Ptr, *nodeUUID90Ptr);

        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID94Ptr, *nodeUUID90Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID94Ptr, *nodeUUID92Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID94Ptr, *nodeUUID96Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID94Ptr, *nodeUUID97Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID94Ptr, *nodeUUID98Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID96Ptr, *nodeUUID95Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID96Ptr, *nodeUUID94Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID98Ptr, *nodeUUID95Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID98Ptr, *nodeUUID94Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID97Ptr, *nodeUUID94Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID92Ptr, *nodeUUID90Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID92Ptr, *nodeUUID94Ptr);
    }

    if (!mNodeUUID.stringUUID().compare("13e5cf8c-5834-4e52-b65b-f9281dd1ff91")) {
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID93Ptr, *nodeUUID95Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID93Ptr, *nodeUUID91Ptr);

        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID95Ptr, *nodeUUID96Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID95Ptr, *nodeUUID98Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID95Ptr, *nodeUUID93Ptr);
    }

    if (!mNodeUUID.stringUUID().compare("13e5cf8c-5834-4e52-b65b-f9281dd1ff94")) {
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID92Ptr, *nodeUUID90Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID92Ptr, *nodeUUID94Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID96Ptr, *nodeUUID95Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID96Ptr, *nodeUUID94Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID98Ptr, *nodeUUID95Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID98Ptr, *nodeUUID94Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID97Ptr, *nodeUUID94Ptr);

        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID90Ptr, *nodeUUID92Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID90Ptr, *nodeUUID94Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID92Ptr, *nodeUUID90Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID92Ptr, *nodeUUID94Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID95Ptr, *nodeUUID93Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID95Ptr, *nodeUUID98Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID95Ptr, *nodeUUID96Ptr);
    }

    if (!mNodeUUID.stringUUID().compare("13e5cf8c-5834-4e52-b65b-f9281dd1ff93")) {
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID95Ptr, *nodeUUID96Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID95Ptr, *nodeUUID98Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID95Ptr, *nodeUUID93Ptr);

        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID96Ptr, *nodeUUID94Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID96Ptr, *nodeUUID95Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID98Ptr, *nodeUUID94Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID98Ptr, *nodeUUID95Ptr);
    }

    delete nodeUUID90Ptr;
    delete nodeUUID91Ptr;
    delete nodeUUID92Ptr;
    delete nodeUUID93Ptr;
    delete nodeUUID94Ptr;
    delete nodeUUID95Ptr;
    delete nodeUUID96Ptr;
    delete nodeUUID97Ptr;
    delete nodeUUID98Ptr;
}

void PathsManager::testStorageHandler()
{
    NodeUUID* nodeUUID81Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff81");
    NodeUUID* nodeUUID82Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff82");
    NodeUUID* nodeUUID83Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff83");
    NodeUUID* nodeUUID84Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff84");
    NodeUUID* nodeUUID85Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff85");
    NodeUUID* nodeUUID86Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff86");
    {
        auto ioTransaction = mStorageHandler->beginTransaction();
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID81Ptr, *nodeUUID82Ptr);
        ioTransaction->rollback();
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID81Ptr, *nodeUUID82Ptr);
    }
    {
        auto ioTransaction = mStorageHandler->beginTransaction();
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID83Ptr, *nodeUUID84Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID85Ptr, *nodeUUID86Ptr);
    }
    {
        auto ioTransaction = mStorageHandler->beginTransaction();
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID82Ptr, *nodeUUID81Ptr);
        ioTransaction->rollback();
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID81Ptr, *nodeUUID83Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID81Ptr, *nodeUUID84Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID83Ptr, *nodeUUID81Ptr);
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    vector<pair<NodeUUID, NodeUUID>> records = ioTransaction->routingTablesHandler()->rt2Records();
    for (auto &record : records) {
        info() << record.first << " " << record.second;
    }

    for (auto const &itMap : ioTransaction->routingTablesHandler()->routeRecordsMapSourceKeyOnRT2()) {
        info() << "key: " << itMap.first;
        for (auto const &itVector : itMap.second) {
            info() << "\tvalue: " << itVector;
        }
    }

    ioTransaction->routingTablesHandler()->removeRecordFromRT2(*nodeUUID81Ptr, *nodeUUID83Ptr);
    ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID81Ptr, *nodeUUID84Ptr);
    info() << "after updating and deleting";
    for (auto const &itMap : ioTransaction->routingTablesHandler()->routeRecordsMapSourceKeyOnRT2()) {
        info() << "key: " << itMap.first;
        for (auto const &itVector : itMap.second) {
            info() << "\tvalue: " << itVector;
        }
    }

    ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID85Ptr, *nodeUUID81Ptr);
    info() << "after updating absent elemnet";
    for (auto const &itMap : ioTransaction->routingTablesHandler()->routeRecordsMapSourceKeyOnRT2()) {
        info() << "key: " << itMap.first;
        for (auto const &itVector : itMap.second) {
            info() << "\tvalue: " << itVector;
        }
    }

    ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID81Ptr, *nodeUUID82Ptr);
    info() << "after updating real elemnet";
    for (auto const &itMap : ioTransaction->routingTablesHandler()->routeRecordsMapSourceKeyOnRT2()) {
        info() << "key: " << itMap.first;
        for (auto const &itVector : itMap.second) {
            info() << "\tvalue: " << itVector;
        }
    }

    info() << "all destinations for source: " << *nodeUUID81Ptr;
    for (const auto &itVect : ioTransaction->routingTablesHandler()->neighborsOfOnRT2(*nodeUUID81Ptr)) {
        info() << "\t\t\t" << itVect;
    }

    delete nodeUUID81Ptr;
    delete nodeUUID82Ptr;
    delete nodeUUID83Ptr;
    delete nodeUUID84Ptr;
    delete nodeUUID85Ptr;
    delete nodeUUID86Ptr;
}

void PathsManager::testTrustLineHandler()
{
    auto ioTransaction = mStorageHandler->beginTransaction();
    for (auto const &trustLine : mTrustLinesManager->trustLines()) {
        ioTransaction->trustLineHandler()->saveTrustLine(trustLine.second);
    }
    TrustLine::Shared trLine = make_shared<TrustLine>(
        mNodeUUID,
        TrustLineAmount(100),
        TrustLineAmount(200),
        TrustLineBalance(-30));
    ioTransaction->trustLineHandler()->saveTrustLine(trLine);
    for (const auto &rtrLine : ioTransaction->trustLineHandler()->allTrustLines()) {
        info() << "read one trust line: " <<
               rtrLine->contractorNodeUUID() << " " <<
               rtrLine->incomingTrustAmount() << " " <<
               rtrLine->outgoingTrustAmount() << " " <<
               rtrLine->balance();
    }
    trLine->setBalance(55);
    trLine->setIncomingTrustAmount(1000);
    ioTransaction->trustLineHandler()->saveTrustLine(trLine);
    for (const auto &rtrLine : ioTransaction->trustLineHandler()->allTrustLines()) {
        info() << "read one trust line: " <<
               rtrLine->contractorNodeUUID() << " " <<
               rtrLine->incomingTrustAmount() << " " <<
               rtrLine->outgoingTrustAmount() << " " <<
               rtrLine->balance();
    }
    ioTransaction->rollback();
    for (const auto &rtrLine : ioTransaction->trustLineHandler()->allTrustLines()) {
        info() << "read one trust line: " <<
               rtrLine->contractorNodeUUID() << " " <<
               rtrLine->incomingTrustAmount() << " " <<
               rtrLine->outgoingTrustAmount() << " " <<
               rtrLine->balance();
    }
    ioTransaction->trustLineHandler()->deleteTrustLine(mNodeUUID);
    for (const auto &rtrLine : ioTransaction->trustLineHandler()->allTrustLines()) {
        info() << "read one trust line: " <<
               rtrLine->contractorNodeUUID() << " " <<
               rtrLine->incomingTrustAmount() << " " <<
               rtrLine->outgoingTrustAmount() << " " <<
               rtrLine->balance();
    }
}

void PathsManager::testPaymentStateOperationsHandler()
{
    auto ioTransaction = mStorageHandler->beginTransaction();
    TransactionUUID transaction1;
    BytesShared state1 = tryMalloc(sizeof(uint8_t));
    uint8_t st1 = 2;
    memcpy(
        state1.get(),
        &st1,
        sizeof(uint8_t));
    ioTransaction->paymentOperationStateHandler()->saveRecord(transaction1, state1, sizeof(uint8_t));
    st1 = 22;
    memcpy(
        state1.get(),
        &st1,
        sizeof(uint8_t));
    try {
        ioTransaction->paymentOperationStateHandler()->saveRecord(transaction1, state1, sizeof(uint8_t));
    } catch (IOError) {
        info() << "record alredy present";
    }
    TransactionUUID transaction2;
    BytesShared state2 = tryMalloc(sizeof(uint16_t));
    uint16_t st2 = 88;
    memcpy(
        state2.get(),
        &st2,
        sizeof(uint16_t));
    ioTransaction->paymentOperationStateHandler()->saveRecord(transaction2, state2, sizeof(uint16_t));
    ioTransaction->commit();
    TransactionUUID transaction3;
    BytesShared state3 = tryMalloc(sizeof(uint32_t));
    uint32_t st3 = 3;
    memcpy(
        state3.get(),
        &st3,
        sizeof(uint32_t));
    ioTransaction->paymentOperationStateHandler()->saveRecord(transaction3, state3, sizeof(uint32_t));
    ioTransaction->rollback();

    pair<BytesShared, size_t> stateBt = ioTransaction->paymentOperationStateHandler()->getState(transaction1);
    uint32_t state = 0;
    memcpy(
        &state,
        stateBt.first.get(),
        stateBt.second);
    info() << stateBt.second << " " << (uint32_t)state;
    try {
        stateBt = ioTransaction->paymentOperationStateHandler()->getState(transaction2);
        memcpy(
            &state,
            stateBt.first.get(),
            stateBt.second);
        info() << stateBt.second << " " << state;
    } catch (NotFoundError) {
        info() << "not found";
    }
    try {
        stateBt = ioTransaction->paymentOperationStateHandler()->getState(transaction3);
        memcpy(
            &state,
            stateBt.first.get(),
            stateBt.second);
        info() << stateBt.second << " " << state;
    } catch (NotFoundError) {
        info() << "not found";
    }
    state1 = tryMalloc(sizeof(uint32_t));
    uint32_t st1_1 = 101;
    memcpy(
        state1.get(),
        &st1_1,
        sizeof(uint32_t));
    ioTransaction->paymentOperationStateHandler()->saveRecord(TransactionUUID(), state1, sizeof(uint32_t));
    try {
        ioTransaction->paymentOperationStateHandler()->saveRecord(transaction3, state3, sizeof(uint32_t));
    } catch (IOError) {
        info() << "record alredy present";
    }
    ioTransaction->paymentOperationStateHandler()->deleteRecord(transaction2);

    info() << "after changes";
    stateBt = ioTransaction->paymentOperationStateHandler()->getState(transaction1);
    memcpy(
        &state,
        stateBt.first.get(),
        stateBt.second);
    info() << stateBt.second << " " << state;
    try {
        stateBt = ioTransaction->paymentOperationStateHandler()->getState(transaction2);
        memcpy(
            &state,
            stateBt.first.get(),
            stateBt.second);
        info() << stateBt.second << " " << state;
    } catch (NotFoundError) {
        info() << "not found";
    }
    try {
        stateBt = ioTransaction->paymentOperationStateHandler()->getState(transaction3);
        memcpy(
            &state,
            stateBt.first.get(),
            stateBt.second);
        info() << stateBt.second << " " << state;
    } catch (NotFoundError) {
        info() << "not found";
    }
    try {
        stateBt = ioTransaction->paymentOperationStateHandler()->getState(TransactionUUID());
        memcpy(
            &state,
            stateBt.first.get(),
            stateBt.second);
        info() << stateBt.second << " " << state;
    } catch (NotFoundError) {
        info() << "not found";
    }
}

void PathsManager::testTransactionHandler()
{
    auto ioTransaction = mStorageHandler->beginTransaction();
    TransactionUUID transaction1;
    BytesShared tr1 = tryMalloc(sizeof(uint8_t));
    uint8_t st1 = 2;
    memcpy(
        tr1.get(),
        &st1,
        sizeof(uint8_t));
    ioTransaction->transactionHandler()->saveRecord(transaction1, tr1, sizeof(uint8_t));
    st1 = 22;
    memcpy(
        tr1.get(),
        &st1,
        sizeof(uint8_t));
    ioTransaction->transactionHandler()->saveRecord(transaction1, tr1, sizeof(uint8_t));
    TransactionUUID transaction2;
    BytesShared tr2 = tryMalloc(sizeof(uint16_t));
    uint16_t st2 = 88;
    memcpy(
        tr2.get(),
        &st2,
        sizeof(uint16_t));
    ioTransaction->transactionHandler()->saveRecord(transaction2, tr2, sizeof(uint16_t));
    ioTransaction->commit();
    TransactionUUID transaction3;
    BytesShared tr3 = tryMalloc(sizeof(uint32_t));
    uint32_t st3 = 3;
    memcpy(
        tr3.get(),
        &st3,
        sizeof(uint32_t));
    ioTransaction->transactionHandler()->saveRecord(transaction3, tr3, sizeof(uint32_t));
    ioTransaction->rollback();

    pair<BytesShared, size_t> stateBt = ioTransaction->transactionHandler()->getTransaction(transaction1);
    uint32_t tr = 0;
    memcpy(
        &tr,
        stateBt.first.get(),
        stateBt.second);
    info() << stateBt.second << " " << (uint32_t)tr;
    try {
        stateBt = ioTransaction->transactionHandler()->getTransaction(transaction2);
        memcpy(
            &tr,
            stateBt.first.get(),
            stateBt.second);
        info() << stateBt.second << " " << tr;
    } catch (NotFoundError) {
        info() << "not found";
    }
    try {
        stateBt = ioTransaction->transactionHandler()->getTransaction(transaction3);
        memcpy(
            &tr,
            stateBt.first.get(),
            stateBt.second);
        info() << stateBt.second << " " << tr;
    } catch (NotFoundError) {
        info() << "not found";
    }
    tr1 = tryMalloc(sizeof(uint32_t));
    uint32_t st1_1 = 101;
    memcpy(
        tr1.get(),
        &st1_1,
        sizeof(uint32_t));
    ioTransaction->transactionHandler()->saveRecord(transaction1, tr1, sizeof(uint32_t));
    ioTransaction->transactionHandler()->saveRecord(transaction3, tr3, sizeof(uint32_t));
    ioTransaction->transactionHandler()->deleteRecord(transaction2);

    info() << "after changes";
    stateBt = ioTransaction->transactionHandler()->getTransaction(transaction1);
    memcpy(
        &tr,
        stateBt.first.get(),
        stateBt.second);
    info() << stateBt.second << " " << tr;
    try {
        stateBt = ioTransaction->transactionHandler()->getTransaction(transaction2);
        memcpy(
            &tr,
            stateBt.first.get(),
            stateBt.second);
        info() << stateBt.second << " " << tr;
    } catch (NotFoundError) {
        info() << "not found";
    }
    try {
        stateBt = ioTransaction->transactionHandler()->getTransaction(transaction3);
        memcpy(
            &tr,
            stateBt.first.get(),
            stateBt.second);
        info() << stateBt.second << " " << tr;
    } catch (NotFoundError) {
        info() << "not found";
    }
    try {
        stateBt = ioTransaction->transactionHandler()->getTransaction(TransactionUUID());
        memcpy(
            &tr,
            stateBt.first.get(),
            stateBt.second);
        info() << stateBt.second << " " << tr;
    } catch (NotFoundError) {
        info() << "not found";
    }
}

void PathsManager::fillBigRoutingTables()
{
    uint32_t firstLevelNode = 20;
    uint32_t countNodes = 5000;
    srand (time(NULL));
    vector<NodeUUID*> nodeUUIDPtrs;
    nodeUUIDPtrs.reserve(countNodes);
    for (uint32_t idx = 1; idx <= countNodes; idx++) {
        nodeUUIDPtrs.push_back(new NodeUUID(nodeUUIDName(idx)));
    }

    uint32_t currentIdx = 0;
    if (!mNodeUUID.stringUUID().compare("13e5cf8c-5834-4e52-b65b-000000000001")) {
        currentIdx = 1;
    }

    if (!mNodeUUID.stringUUID().compare("13e5cf8c-5834-4e52-b65b-000000000002")) {
        currentIdx = 2;
    }

    vector<uint32_t> firstLevelNodes;
    auto ioTransaction = mStorageHandler->beginTransaction();
    while (firstLevelNodes.size() < firstLevelNode) {
        uint32_t  nextIdx = rand() % countNodes + 1;
        if(std::find(firstLevelNodes.begin(), firstLevelNodes.end(), nextIdx) != firstLevelNodes.end() ||
           nextIdx == currentIdx) {
            continue;
        }
        firstLevelNodes.push_back(nextIdx);
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *getPtrByNodeNumber(
                    nextIdx,
                    nodeUUIDPtrs),
                TrustLineAmount(200),
                TrustLineAmount(200),
                TrustLineBalance(0)));
    }
    info() << "PathsManager::fillBigRoutingTables first level done";
    set<uint32_t> secondLevelAllNodes;
    for (auto firstLevelIdx : firstLevelNodes) {
        vector<uint32_t> secondLevelNodes;
        while (secondLevelNodes.size() < firstLevelNode) {
            uint32_t  nextIdx = rand() % countNodes + 1;
            if(std::find(secondLevelNodes.begin(), secondLevelNodes.end(), nextIdx) != secondLevelNodes.end() ||
               nextIdx == firstLevelIdx) {
                continue;
            }
            secondLevelNodes.push_back(nextIdx);
            secondLevelAllNodes.insert(nextIdx);
            ioTransaction->routingTablesHandler()->setRecordToRT2(
                *getPtrByNodeNumber(
                    firstLevelIdx,
                    nodeUUIDPtrs),
                *getPtrByNodeNumber(
                    nextIdx,
                    nodeUUIDPtrs));
        }
        //info() << "fillBigRoutingTables:: second level commit";
    }
    info() << "fillBigRoutingTables:: second level done";
    for (auto secondLevelIdx : secondLevelAllNodes) {
        vector<uint32_t> thirdLevelNodes;
        while (thirdLevelNodes.size() < firstLevelNode) {
            uint32_t  nextIdx = rand() % countNodes + 1;
            if(std::find(thirdLevelNodes.begin(), thirdLevelNodes.end(), nextIdx) != thirdLevelNodes.end() ||
               nextIdx == secondLevelIdx) {
                continue;
            }
            thirdLevelNodes.push_back(nextIdx);
            ioTransaction->routingTablesHandler()->setRecordToRT3(
                *getPtrByNodeNumber(
                    secondLevelIdx,
                    nodeUUIDPtrs),
                *getPtrByNodeNumber(
                    nextIdx,
                    nodeUUIDPtrs));
        }
        //info() << "fillBigRoutingTables:: third level commit";
    }
    info() << "fillBigRoutingTables:: third level done";
    for (auto nodeUUIDPrt : nodeUUIDPtrs) {
        delete nodeUUIDPrt;
    }
}

string PathsManager::nodeUUIDName(uint32_t number)
{
    stringstream s;
    s << number;
    string numStr = s.str();
    while (numStr.length() < 12) {
        numStr = "0" + numStr;
    }
    return "13e5cf8c-5834-4e52-b65b-" + numStr;
}

NodeUUID* PathsManager::getPtrByNodeNumber(
    uint32_t number,
    vector<NodeUUID*> nodeUUIDPtrs)
{
    string nodeUUIDStr = nodeUUIDName(number);
    for (auto nodeUUIDPtr : nodeUUIDPtrs) {
        if (nodeUUIDStr.compare(nodeUUIDPtr->stringUUID()) == 0) {
            return nodeUUIDPtr;
        }
    }
}

void PathsManager::testTime()
{
    auto ioTransaction = mStorageHandler->beginTransaction();
    info() << "testTime\t" << "RT2 size: " << ioTransaction->routingTablesHandler()->rt2Records().size();
    info() << "testTime\t" << "RT2 map size opt5: " << ioTransaction->routingTablesHandler()->routeRecordsMapDestinationKeyOnRT2().size();

    info() << "testTime\t" << "RT3 size: " << ioTransaction->routingTablesHandler()->routeRecordsMapSourceKeyOnRT3().size();
    info() << "testTime\t" << "RT3 map size opt5: " << ioTransaction->routingTablesHandler()->routeRecordsMapDestinationKeyOnRT3().size();
}

void PathsManager::testMultiConnection()
{
    string queryCreateTable = "CREATE TABLE IF NOT EXISTS test_table "
        "(field1 INTEGER NOT NULL, "
        "field2 INTEGER NOT NULL);";
    string queryBegin = "BEGIN TRANSACTION;";
    string queryInsert = "INSERT INTO test_table (field1, field2) VALUES (?, ?);";
    string queryCommit = "END TRANSACTION;";
    string selectQuery = "SELECT * FROM test_table";

    sqlite3_stmt *stmt1;
    sqlite3_stmt *stmt2;

    sqlite3 *database1;
    sqlite3 *database2;

    int rc = sqlite3_open_v2("io/storageDB", &database1, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (rc == SQLITE_OK) {
    } else {
        throw IOError("PathsManager::testMultiConnection "
                          "Can't open database 1");
    }

    // create table conn1
    rc = sqlite3_prepare_v2( database1, queryCreateTable.c_str(), -1, &stmt1, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PathsManager::testMultiConnection 1 creating table : Bad query");
    }
    rc = sqlite3_step(stmt1);
    if (rc == SQLITE_DONE) {
    } else {
        info() << "testMultiConnection 1 creating table error: " << rc;
        throw IOError("PathsManager::testMultiConnection 1 creating table : Run query");
    }
    sqlite3_reset(stmt1);
    sqlite3_finalize(stmt1);


    rc = sqlite3_open_v2("io/storageDB", &database2, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (rc == SQLITE_OK) {
    } else {
        throw IOError("PathsManager::testMultiConnection "
                          "Can't open database 2");
    }

    // create table conn2
    rc = sqlite3_prepare_v2( database2, queryCreateTable.c_str(), -1, &stmt2, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PathsManager::testMultiConnection 2 creating table : Bad query");
    }
    rc = sqlite3_step(stmt2);
    if (rc == SQLITE_DONE) {
    } else {
        throw IOError("PathsManager::testMultiConnection 2 creating table : Run query");
    }
    sqlite3_reset(stmt2);
    sqlite3_finalize(stmt2);

    // transaction 1 begin
    rc = sqlite3_prepare_v2( database1, queryBegin.c_str(), -1, &stmt1, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PathsManager::testMultiConnection 1 prepareInserted: Bad query");
    }
    rc = sqlite3_step(stmt1);
    if (rc == SQLITE_DONE) {
        info() << "testMultiConnection 1 transaction begin";
    } else {
        info() << "testMultiConnection 1 prepareInserted error: " << rc;
        //throw IOError("PathsManager::testMultiConnection 1 prepareInserted: Run query");
    }
    sqlite3_reset(stmt1);
    sqlite3_finalize(stmt1);

    // transaction 1 insert
    rc = sqlite3_prepare_v2(database1, queryInsert.c_str(), -1, &stmt1, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PathsManager::testMultiConnection 1 insert: Bad query");
    }
    rc = sqlite3_bind_int(stmt1, 1, 1);
    if (rc != SQLITE_OK) {
        throw IOError("PathsManager::testMultiConnection 1 insert: Bad binding of field1");
    }
    rc = sqlite3_bind_int(stmt1, 2, 11);
    if (rc != SQLITE_OK) {
        throw IOError("PathsManager::testMultiConnection 1 insert: Bad binding of field2");
    }
    rc = sqlite3_step(stmt1);
    if (rc == SQLITE_DONE) {
        info() << "testMultiConnection 1 inserting is completed successfully";
    } else {
        info() << "testMultiConnection 1 insert error: " << rc;
        //throw IOError("PathsManager::testMultiConnection 1 insert: Run query");
    }
    sqlite3_reset(stmt1);
    sqlite3_finalize(stmt1);

    // transaction 1 select
    rc = sqlite3_prepare_v2(database1, selectQuery.c_str(), -1, &stmt1, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PathsManager::testMultiConnection 1 select: Bad query");
    }
    while (sqlite3_step(stmt1) == SQLITE_ROW) {
        info() << "testMultiConnection 1 select " << sqlite3_column_int(stmt1, 0) << " " << sqlite3_column_int(stmt1, 0);
    }
    sqlite3_reset(stmt1);
    sqlite3_finalize(stmt1);

    // transaction 2 begin
    rc = sqlite3_prepare_v2( database2, queryBegin.c_str(), -1, &stmt2, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PathsManager::testMultiConnection 2 prepareInserted: Bad query");
    }
    rc = sqlite3_step(stmt2);
    if (rc == SQLITE_DONE) {
        info() << "testMultiConnection 2 transaction begin";
    } else {
        info() << "testMultiConnection 2 prepareInserted error: " << rc;
        //throw IOError("PathsManager::testMultiConnection 2 prepareInserted: Run query");
    }
    sqlite3_reset(stmt2);
    sqlite3_finalize(stmt2);

    // transaction 2 insert
    rc = sqlite3_prepare_v2(database2, queryInsert.c_str(), -1, &stmt2, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PathsManager::testMultiConnection 2 insert: Bad query");
    }
    rc = sqlite3_bind_int(stmt2, 1, 2);
    if (rc != SQLITE_OK) {
        throw IOError("PathsManager::testMultiConnection 2 insert: Bad binding of field1");
    }
    rc = sqlite3_bind_int(stmt2, 2, 22);
    if (rc != SQLITE_OK) {
        throw IOError("PathsManager::testMultiConnection 2 insert: Bad binding of field2");
    }
    rc = sqlite3_step(stmt2);
    if (rc == SQLITE_DONE) {
        info() << "testMultiConnection 2 inserting is completed successfully";
    } else {
        info() << "testMultiConnection 2 insert error: " << rc;
        //throw IOError("PathsManager::testMultiConnection 2 insert: Run query");
    }
    sqlite3_reset(stmt2);
    sqlite3_finalize(stmt2);

    // transaction 2 select
    rc = sqlite3_prepare_v2(database2, selectQuery.c_str(), -1, &stmt2, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PathsManager::testMultiConnection 2 select: Bad query");
    }
    while (sqlite3_step(stmt2) == SQLITE_ROW) {
        info() << "testMultiConnection 2 select " << sqlite3_column_int(stmt2, 0) << " " << sqlite3_column_int(stmt2, 0);
    }
    sqlite3_reset(stmt2);
    sqlite3_finalize(stmt2);

    // transaction 1 commit
    rc = sqlite3_prepare_v2( database1, queryCommit.c_str(), -1, &stmt1, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PathsManager::testMultiConnection 1 commit: Bad query");
    }
    rc = sqlite3_step(stmt1);
    if (rc == SQLITE_DONE) {
        info() << "testMultiConnection 1 transaction commit";
    } else {
        info() << "testMultiConnection 1 commit error: " << rc;
        //throw IOError("PathsManager::testMultiConnection 1 commit: Run query");
    }
    sqlite3_reset(stmt1);
    sqlite3_finalize(stmt1);

    // transaction 2 commit
    rc = sqlite3_prepare_v2( database2, queryCommit.c_str(), -1, &stmt2, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PathsManager::testMultiConnection 2 commit: Bad query");
    }
    rc = sqlite3_step(stmt2);
    if (rc == SQLITE_DONE) {
        info() << "testMultiConnection 2 transaction commit";
    } else {
        info() << "testMultiConnection 2 commit error: " << rc;
        //throw IOError("PathsManager::testMultiConnection 2 commit: Run query");
    }
    sqlite3_reset(stmt2);
    sqlite3_finalize(stmt2);

    // transaction 1 select
    rc = sqlite3_prepare_v2(database1, selectQuery.c_str(), -1, &stmt1, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PathsManager::testMultiConnection 1 select: Bad query");
    }
    while (sqlite3_step(stmt1) == SQLITE_ROW) {
        info() << "testMultiConnection 1 select " << sqlite3_column_int(stmt1, 0) << " " << sqlite3_column_int(stmt1, 0);
    }
    sqlite3_reset(stmt1);
    sqlite3_finalize(stmt1);

    // transaction 2 select
    rc = sqlite3_prepare_v2(database2, selectQuery.c_str(), -1, &stmt2, 0);
    if (rc != SQLITE_OK) {
        throw IOError("PathsManager::testMultiConnection 2 select: Bad query");
    }
    while (sqlite3_step(stmt2) == SQLITE_ROW) {
        info() << "testMultiConnection 2 select " << sqlite3_column_int(stmt2, 0) << " " << sqlite3_column_int(stmt2, 0);
    }
    sqlite3_reset(stmt2);
    sqlite3_finalize(stmt2);

    sqlite3_close_v2(database1);
    sqlite3_close_v2(database2);
}

void PathsManager::printRTs()
{
    auto ioTransaction = mStorageHandler->beginTransaction();
    info() << "printRTs\tRT1 size: " << mTrustLinesManager->trustLines().size();
    for (const auto itTrustLine : mTrustLinesManager->trustLines()) {
        info() << "printRTs\t" << itTrustLine.second->contractorNodeUUID() << " "
               << itTrustLine.second->incomingTrustAmount() << " "
               << itTrustLine.second->outgoingTrustAmount() << " "
               << itTrustLine.second->balance();
    }
    info() << "printRTs\tRT2 size: " << ioTransaction->routingTablesHandler()->rt2Records().size();
    for (auto const itRT2 : ioTransaction->routingTablesHandler()->rt2Records()) {
        info() << itRT2.first << " " << itRT2.second;
    }
    info() << "printRTs\tRT3 size: " << ioTransaction->routingTablesHandler()->rt3Records().size();
    for (auto const itRT3 : ioTransaction->routingTablesHandler()->rt3Records()) {
        info() << itRT3.first << " " << itRT3.second;
    }
}

void PathsManager::testDeletingRT()
{
    NodeUUID* nodeUUID81Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff81");
    NodeUUID* nodeUUID82Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff82");
    NodeUUID* nodeUUID83Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff83");
    NodeUUID* nodeUUID84Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff84");
    NodeUUID* nodeUUID85Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff85");
    NodeUUID* nodeUUID86Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff86");
    {
        auto ioTransaction = mStorageHandler->beginTransaction();
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID81Ptr, *nodeUUID82Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID83Ptr, *nodeUUID82Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID83Ptr, *nodeUUID81Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT2(*nodeUUID81Ptr, *nodeUUID83Ptr);

        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID82Ptr, *nodeUUID84Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID85Ptr, *nodeUUID86Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID82Ptr, *nodeUUID81Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID81Ptr, *nodeUUID83Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID83Ptr, *nodeUUID84Ptr);
        ioTransaction->routingTablesHandler()->setRecordToRT3(*nodeUUID82Ptr, *nodeUUID86Ptr);
    }

    vector<pair<NodeUUID, NodeUUID>> records;
    {
        auto ioTransaction = mStorageHandler->beginTransaction();
        info() << "RT2:";
        records = ioTransaction->routingTablesHandler()->rt2Records();
        for (auto &record : records) {
            info() << record.first << " " << record.second;
        }
        info() << "RT3:";
        records = ioTransaction->routingTablesHandler()->rt3Records();
        for (auto &record : records) {
            info() << record.first << " " << record.second;
        }
    }

    {
        auto ioTransaction = mStorageHandler->beginTransaction();
        ioTransaction->routingTablesHandler()->removeRecordFromRT2(*nodeUUID83Ptr, *nodeUUID82Ptr);
        info() << "after first deleting " << *nodeUUID83Ptr << " " << *nodeUUID82Ptr;
        info() << "RT2:";
        records = ioTransaction->routingTablesHandler()->rt2Records();
        for (auto &record : records) {
            info() << record.first << " " << record.second;
        }
        info() << "RT3:";
        records = ioTransaction->routingTablesHandler()->rt3Records();
        for (auto &record : records) {
            info() << record.first << " " << record.second;
        }
    }

    {
        auto ioTransaction = mStorageHandler->beginTransaction();
        ioTransaction->routingTablesHandler()->removeRecordFromRT2(*nodeUUID81Ptr, *nodeUUID83Ptr);
        info() << "after second deleting " << *nodeUUID81Ptr << " " << *nodeUUID83Ptr;
        info() << "RT2:";
        records = ioTransaction->routingTablesHandler()->rt2Records();
        for (auto &record : records) {
            info() << record.first << " " << record.second;
        }
        info() << "RT3:";
        records = ioTransaction->routingTablesHandler()->rt3Records();
        for (auto &record : records) {
            info() << record.first << " " << record.second;
        }
    }

    {
        auto ioTransaction = mStorageHandler->beginTransaction();
        ioTransaction->routingTablesHandler()->removeRecordFromRT2(*nodeUUID81Ptr, *nodeUUID82Ptr);
        info() << "after third deleting " << *nodeUUID81Ptr << " " << *nodeUUID82Ptr;
        info() << "RT2:";
        records = ioTransaction->routingTablesHandler()->rt2Records();
        for (auto &record : records) {
            info() << record.first << " " << record.second;
        }
        info() << "RT3:";
        records = ioTransaction->routingTablesHandler()->rt3Records();
        for (auto &record : records) {
            info() << record.first << " " << record.second;
        }
    }

    delete nodeUUID81Ptr;
    delete nodeUUID82Ptr;
    delete nodeUUID83Ptr;
    delete nodeUUID84Ptr;
    delete nodeUUID85Ptr;
    delete nodeUUID86Ptr;
}

void PathsManager::fillCycleTablesTestCase0()
{
    NodeUUID *nodeUUID51Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff51");
    NodeUUID *nodeUUID52Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff52");
    NodeUUID *nodeUUID53Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff53");
    NodeUUID *nodeUUID54Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff54");
    NodeUUID *nodeUUID55Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff55");

    auto ioTransaction = mStorageHandler->beginTransaction();
    if (mNodeUUID == *nodeUUID51Ptr) {
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID52Ptr,
                TrustLineAmount(200),
                TrustLineAmount(0),
                TrustLineBalance(0)));
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID55Ptr,
                TrustLineAmount(0),
                TrustLineAmount(190),
                TrustLineBalance(0)));
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID53Ptr,
                TrustLineAmount(0),
                TrustLineAmount(100),
                TrustLineBalance(0)));
    }

    if (mNodeUUID == *nodeUUID52Ptr) {
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID53Ptr,
                TrustLineAmount(180),
                TrustLineAmount(0),
                TrustLineBalance(0)));
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID51Ptr,
                TrustLineAmount(0),
                TrustLineAmount(200),
                TrustLineBalance(0)));
    }

    if (mNodeUUID == *nodeUUID53Ptr) {
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID54Ptr,
                TrustLineAmount(150),
                TrustLineAmount(0),
                TrustLineBalance(0)));
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID51Ptr,
                TrustLineAmount(100),
                TrustLineAmount(0),
                TrustLineBalance(0)));
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID52Ptr,
                TrustLineAmount(0),
                TrustLineAmount(180),
                TrustLineBalance(0)));
    }

    if (mNodeUUID == *nodeUUID54Ptr) {
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID55Ptr,
                TrustLineAmount(200),
                TrustLineAmount(0),
                TrustLineBalance(0)));
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID53Ptr,
                TrustLineAmount(0),
                TrustLineAmount(150),
                TrustLineBalance(0)));
    }

    if (mNodeUUID == *nodeUUID55Ptr) {
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID51Ptr,
                TrustLineAmount(190),
                TrustLineAmount(0),
                TrustLineBalance(0)));
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID54Ptr,
                TrustLineAmount(0),
                TrustLineAmount(200),
                TrustLineBalance(0)));
    }

    delete nodeUUID51Ptr;
    delete nodeUUID52Ptr;
    delete nodeUUID53Ptr;
    delete nodeUUID54Ptr;
    delete nodeUUID55Ptr;
}

void PathsManager::fillCycleTablesTestCase1()
{
    NodeUUID *nodeUUID51Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff51");
    NodeUUID *nodeUUID52Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff52");
    NodeUUID *nodeUUID53Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff53");
    NodeUUID *nodeUUID54Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff54");
    NodeUUID *nodeUUID55Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff55");
    NodeUUID *nodeUUID56Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff56");

    auto ioTransaction = mStorageHandler->beginTransaction();
    if (mNodeUUID == *nodeUUID51Ptr) {
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID53Ptr,
                TrustLineAmount(180),
                TrustLineAmount(0),
                TrustLineBalance(0)));
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID55Ptr,
                TrustLineAmount(150),
                TrustLineAmount(0),
                TrustLineBalance(0)));
    }

    if (mNodeUUID == *nodeUUID52Ptr) {
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID54Ptr,
                TrustLineAmount(0),
                TrustLineAmount(180),
                TrustLineBalance(0)));
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID56Ptr,
                TrustLineAmount(0),
                TrustLineAmount(50),
                TrustLineBalance(0)));
    }

    if (mNodeUUID == *nodeUUID53Ptr) {
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID54Ptr,
                TrustLineAmount(150),
                TrustLineAmount(0),
                TrustLineBalance(0)));
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID51Ptr,
                TrustLineAmount(0),
                TrustLineAmount(180),
                TrustLineBalance(0)));
    }

    if (mNodeUUID == *nodeUUID54Ptr) {
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID52Ptr,
                TrustLineAmount(180),
                TrustLineAmount(0),
                TrustLineBalance(0)));
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID53Ptr,
                TrustLineAmount(0),
                TrustLineAmount(150),
                TrustLineBalance(0)));
    }

    if (mNodeUUID == *nodeUUID55Ptr) {
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID56Ptr,
                TrustLineAmount(100),
                TrustLineAmount(0),
                TrustLineBalance(0)));
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID51Ptr,
                TrustLineAmount(0),
                TrustLineAmount(150),
                TrustLineBalance(0)));
    }

    if (mNodeUUID == *nodeUUID56Ptr) {
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID52Ptr,
                TrustLineAmount(50),
                TrustLineAmount(0),
                TrustLineBalance(0)));
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID55Ptr,
                TrustLineAmount(0),
                TrustLineAmount(100),
                TrustLineBalance(0)));
    }

    delete nodeUUID51Ptr;
    delete nodeUUID52Ptr;
    delete nodeUUID53Ptr;
    delete nodeUUID54Ptr;
    delete nodeUUID55Ptr;
    delete nodeUUID56Ptr;
}

void PathsManager::fillCycleTablesTestCase2()
{
    NodeUUID *nodeUUID51Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff51");
    NodeUUID *nodeUUID52Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff52");
    NodeUUID *nodeUUID53Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff53");
    NodeUUID *nodeUUID54Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff54");
    NodeUUID *nodeUUID55Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff55");

    auto ioTransaction = mStorageHandler->beginTransaction();
    if (mNodeUUID == *nodeUUID51Ptr) {
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID52Ptr,
                TrustLineAmount(300),
                TrustLineAmount(0),
                TrustLineBalance(0)));
    }

    if (mNodeUUID == *nodeUUID52Ptr) {
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID53Ptr,
                TrustLineAmount(180),
                TrustLineAmount(0),
                TrustLineBalance(0)));
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID54Ptr,
                TrustLineAmount(100),
                TrustLineAmount(0),
                TrustLineBalance(0)));
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID51Ptr,
                TrustLineAmount(0),
                TrustLineAmount(300),
                TrustLineBalance(0)));
    }

    if (mNodeUUID == *nodeUUID53Ptr) {
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID55Ptr,
                TrustLineAmount(150),
                TrustLineAmount(0),
                TrustLineBalance(0)));
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID52Ptr,
                TrustLineAmount(0),
                TrustLineAmount(180),
                TrustLineBalance(0)));
    }

    if (mNodeUUID == *nodeUUID54Ptr) {
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID55Ptr,
                TrustLineAmount(50),
                TrustLineAmount(0),
                TrustLineBalance(0)));
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID52Ptr,
                TrustLineAmount(0),
                TrustLineAmount(100),
                TrustLineBalance(0)));
    }

    if (mNodeUUID == *nodeUUID55Ptr) {
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID53Ptr,
                TrustLineAmount(0),
                TrustLineAmount(150),
                TrustLineBalance(0)));
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID54Ptr,
                TrustLineAmount(0),
                TrustLineAmount(50),
                TrustLineBalance(0)));
    }

    delete nodeUUID51Ptr;
    delete nodeUUID52Ptr;
    delete nodeUUID53Ptr;
    delete nodeUUID54Ptr;
    delete nodeUUID55Ptr;
}

void PathsManager::fillCycleTablesTestCase3()
{
    NodeUUID *nodeUUID51Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff51");
    NodeUUID *nodeUUID52Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff52");
    NodeUUID *nodeUUID53Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff53");
    NodeUUID *nodeUUID54Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff54");
    NodeUUID *nodeUUID55Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff55");
    NodeUUID *nodeUUID56Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff56");

    auto ioTransaction = mStorageHandler->beginTransaction();
    if (mNodeUUID == *nodeUUID51Ptr) {
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID52Ptr,
                TrustLineAmount(200),
                TrustLineAmount(0),
                TrustLineBalance(0)));
    }

    if (mNodeUUID == *nodeUUID52Ptr) {
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID53Ptr,
                TrustLineAmount(180),
                TrustLineAmount(0),
                TrustLineBalance(0)));
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID54Ptr,
                TrustLineAmount(100),
                TrustLineAmount(0),
                TrustLineBalance(0)));
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID51Ptr,
                TrustLineAmount(0),
                TrustLineAmount(200),
                TrustLineBalance(0)));
    }

    if (mNodeUUID == *nodeUUID53Ptr) {
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID55Ptr,
                TrustLineAmount(150),
                TrustLineAmount(0),
                TrustLineBalance(0)));
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID52Ptr,
                TrustLineAmount(0),
                TrustLineAmount(180),
                TrustLineBalance(0)));
    }

    if (mNodeUUID == *nodeUUID54Ptr) {
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID55Ptr,
                TrustLineAmount(50),
                TrustLineAmount(0),
                TrustLineBalance(0)));
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID52Ptr,
                TrustLineAmount(0),
                TrustLineAmount(100),
                TrustLineBalance(0)));
    }

    if (mNodeUUID == *nodeUUID55Ptr) {
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID56Ptr,
                TrustLineAmount(200),
                TrustLineAmount(0),
                TrustLineBalance(0)));
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID53Ptr,
                TrustLineAmount(0),
                TrustLineAmount(150),
                TrustLineBalance(0)));
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID54Ptr,
                TrustLineAmount(0),
                TrustLineAmount(50),
                TrustLineBalance(0)));
    }

    if (mNodeUUID == *nodeUUID56Ptr) {
        ioTransaction->trustLineHandler()->saveTrustLine(
            make_shared<TrustLine>(
                *nodeUUID55Ptr,
                TrustLineAmount(0),
                TrustLineAmount(200),
                TrustLineBalance(0)));
    }

    delete nodeUUID51Ptr;
    delete nodeUUID52Ptr;
    delete nodeUUID53Ptr;
    delete nodeUUID54Ptr;
    delete nodeUUID55Ptr;
    delete nodeUUID56Ptr;
}

LoggerStream PathsManager::info() const
{
    if (nullptr == mLog)
        throw Exception("logger is not initialised");
    return mLog->info(logHeader());
}

const string PathsManager::logHeader() const
{
    stringstream s;
    s << "[PathsManager]";
    return s.str();
}