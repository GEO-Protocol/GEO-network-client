#include "PathsManager.h"

PathsManager::PathsManager(
    const NodeUUID &nodeUUID,
    TrustLinesManager *trustLinesManager,
    StorageHandler *storageHandler,
    MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
    Logger &logger):

    mNodeUUID(nodeUUID),
    mTrustLinesManager(trustLinesManager),
    mStorageHandler(storageHandler),
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager),
    mLog(logger),
    mPathCollection(nullptr)
{}

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

TrustLineAmount PathsManager::buildPaths(
    const NodeUUID &contractorUUID)
{
    info() << "Build paths to " << contractorUUID;
    mContractorUUID = contractorUUID;
    TrustLineAmount result = 0;
    mPathCollection = make_shared<PathsCollection>(
        mNodeUUID,
        mContractorUUID);

    auto trustLinePtrsSet =
        mMaxFlowCalculationTrustLineManager->trustLinePtrsSet(mNodeUUID);
    if (trustLinePtrsSet.size() == 0) {
        mMaxFlowCalculationTrustLineManager->resetAllUsedAmounts();
        return result;
    }

    for (mCurrentPathLength = 1; mCurrentPathLength <= kMaxPathLength; mCurrentPathLength++) {
        result += buildPathsOnOneLevel();
    }

    mMaxFlowCalculationTrustLineManager->resetAllUsedAmounts();
    return result;
}

TrustLineAmount PathsManager::buildPathsOnOneLevel()
{
    TrustLineAmount result = 0;
    auto trustLinePtrsSet =
        mMaxFlowCalculationTrustLineManager->trustLinePtrsSet(mNodeUUID);
    while(true) {
        TrustLineAmount currentFlow = 0;
        for (auto &trustLinePtr : trustLinePtrsSet) {
            auto trustLine = trustLinePtr->maxFlowCalculationtrustLine();
            auto trustLineFreeAmountShared = trustLine.get()->freeAmount();
            auto trustLineAmountPtr = trustLineFreeAmountShared.get();
            passedNodeUUIDs.clear();
            TrustLineAmount flow = calculateOneNode(
                trustLine.get()->targetUUID(),
                *trustLineAmountPtr,
                1);
            if (flow > TrustLine::kZeroAmount()) {
                currentFlow += flow;
                trustLine->addUsedAmount(flow);
                break;
            }
        }
        result += currentFlow;
        if (currentFlow == 0) {
            break;
        }
    }
    return result;
}

TrustLineAmount PathsManager::calculateOneNode(
    const NodeUUID& nodeUUID,
    const TrustLineAmount& currentFlow,
    byte level)
{
    if (nodeUUID == mContractorUUID) {
        if (currentFlow > TrustLine::kZeroAmount()) {
            Path path(
                mNodeUUID,
                mContractorUUID,
                passedNodeUUIDs);
            mPathCollection->add(path);
            info() << "build path: " << path.toString() << " with amount " << currentFlow;
        }
        return currentFlow;
    }
    if (level == mCurrentPathLength) {
        return 0;
    }

    auto trustLinePtrsSet =
            mMaxFlowCalculationTrustLineManager->trustLinePtrsSet(nodeUUID);
    if (trustLinePtrsSet.size() == 0) {
        return 0;
    }
    for (auto &trustLinePtr : trustLinePtrsSet) {
        auto trustLine = trustLinePtr->maxFlowCalculationtrustLine();
        if (trustLine.get()->targetUUID() == mNodeUUID) {
            continue;
        }
        if (find(
                passedNodeUUIDs.begin(),
                passedNodeUUIDs.end(),
                trustLine.get()->targetUUID()) != passedNodeUUIDs.end()) {
            continue;
        }
        TrustLineAmount nextFlow = currentFlow;
        auto trustLineFreeAmountShared = trustLine.get()->freeAmount();
        auto trustLineFreeAmountPtr = trustLineFreeAmountShared.get();
        if (*trustLineFreeAmountPtr < currentFlow) {
            nextFlow = *trustLineFreeAmountPtr;
        }
        if (nextFlow == TrustLine::kZeroAmount()) {
            continue;
        }
        passedNodeUUIDs.push_back(nodeUUID);
        TrustLineAmount calcFlow = calculateOneNode(
            trustLine.get()->targetUUID(),
            nextFlow,
            level + (byte)1);
        passedNodeUUIDs.pop_back();
        if (calcFlow > TrustLine::kZeroAmount()) {
            trustLine->addUsedAmount(calcFlow);
            return calcFlow;
        }
    }
    return 0;
}

PathsCollection::Shared PathsManager::pathCollection() const
{
    return mPathCollection;
}

void PathsManager::clearPathsCollection()
{
    mPathCollection = nullptr;
}

LoggerStream PathsManager::info() const
{
    return mLog.info(logHeader());
}

const string PathsManager::logHeader() const
{
    return "[PathsManager]";
}