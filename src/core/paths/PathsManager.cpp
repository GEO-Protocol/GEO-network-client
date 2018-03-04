#include "PathsManager.h"

PathsManager::PathsManager(
    const SerializedEquivalent equivalent,
    const NodeUUID &nodeUUID,
    TrustLinesManager *trustLinesManager,
    TopologyTrustLinesManager *topologyTrustLineManager,
    Logger &logger):

    mEquivalent(equivalent),
    mNodeUUID(nodeUUID),
    mTrustLinesManager(trustLinesManager),
    mTopologyTrustLineManager(topologyTrustLineManager),
    mLog(logger),
    mPathCollection(nullptr)
{}

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

// this method used the same logic as PathsManager::buildPaths
void PathsManager::buildPaths(
    const NodeUUID &contractorUUID)
{
    info() << "Build paths to " << contractorUUID;
    auto startTime = utc_now();
    mTopologyTrustLineManager->makeFullyUsedTLsFromGatewaysToAllNodesExceptOne(
        contractorUUID);
    mContractorUUID = contractorUUID;
    mPathCollection = make_shared<PathsCollection>(
        mNodeUUID,
        mContractorUUID);

    auto trustLinePtrsSet =
        mTopologyTrustLineManager->trustLinePtrsSet(mNodeUUID);
    if (trustLinePtrsSet.empty()) {
        mTopologyTrustLineManager->resetAllUsedAmounts();
        return;
    }

    mCurrentPathLength = 1;
    buildPathsOnOneLevel();
    mCurrentPathLength = 2;
    buildPathsOnSecondLevel();
    for (mCurrentPathLength = 3; mCurrentPathLength <= kMaxPathLength; mCurrentPathLength++) {
        buildPathsOnOneLevel();
    }

    mTopologyTrustLineManager->resetAllUsedAmounts();
    info() << "building time: " << utc_now() - startTime;
}

// this method used the same logic as PathsManager::reBuildPathsOnOneLevel
// and InitiateMaxFlowCalculationTransaction::calculateMaxFlowOnOneLevel
void PathsManager::buildPathsOnOneLevel()
{
    auto trustLinePtrsSet =
        mTopologyTrustLineManager->trustLinePtrsSet(mNodeUUID);
    auto itTrustLinePtr = trustLinePtrsSet.begin();
    while (itTrustLinePtr != trustLinePtrsSet.end()) {
        auto trustLine = (*itTrustLinePtr)->topologyTrustLine();
        auto trustLineFreeAmountShared = trustLine->freeAmount();
        auto trustLineAmountPtr = trustLineFreeAmountShared.get();
        if (*trustLineAmountPtr == TrustLine::kZeroAmount()) {
            itTrustLinePtr++;
            continue;
        }
        mPassedNodeUUIDs.clear();
        TrustLineAmount flow = calculateOneNode(
            trustLine->targetUUID(),
            *trustLineAmountPtr,
            1);
        if (flow > TrustLine::kZeroAmount()) {
            trustLine->addUsedAmount(flow);
        } else {
            itTrustLinePtr++;
        }
    }
}

// on second level (paths on 3 nodes) we build paths through gateway first of all
void PathsManager::buildPathsOnSecondLevel()
{
    auto trustLinePtrsSet =
            mTopologyTrustLineManager->trustLinePtrsSet(mNodeUUID);
    auto gateways = mTrustLinesManager->gateways();
    while (!gateways.empty()) {
        auto itGateway = gateways.begin();
        for (auto itTrustLinePtr = trustLinePtrsSet.begin(); itTrustLinePtr != trustLinePtrsSet.end(); itTrustLinePtr++) {
            auto trustLine = (*itTrustLinePtr)->topologyTrustLine();
            bool isContinue = true;
            if (trustLine->targetUUID() == *itGateway) {
                auto trustLineFreeAmountShared = trustLine->freeAmount();
                auto trustLineAmountPtr = trustLineFreeAmountShared.get();
                mPassedNodeUUIDs.clear();
                TrustLineAmount flow = calculateOneNode(
                    trustLine->targetUUID(),
                    *trustLineAmountPtr,
                    1);
                if (flow > TrustLine::kZeroAmount()) {
                    trustLine->addUsedAmount(flow);
                }
                isContinue = false;
            }
            if (!isContinue) {
                trustLinePtrsSet.erase(itTrustLinePtr);
                break;
            }
        }
        gateways.erase(itGateway);

    }
    auto itTrustLinePtr = trustLinePtrsSet.begin();
    while (itTrustLinePtr != trustLinePtrsSet.end()) {
        auto trustLine = (*itTrustLinePtr)->topologyTrustLine();
        auto trustLineFreeAmountShared = trustLine->freeAmount();
        auto trustLineAmountPtr = trustLineFreeAmountShared.get();
        mPassedNodeUUIDs.clear();
        TrustLineAmount flow = calculateOneNode(
            trustLine->targetUUID(),
            *trustLineAmountPtr,
            1);
        if (flow > TrustLine::kZeroAmount()) {
            trustLine->addUsedAmount(flow);
        } else {
            itTrustLinePtr++;
        }
    }
}

// it used the same logic as PathsManager::calculateOneNodeForRebuildingPaths
// and InitiateMaxFlowCalculationTransaction::calculateOneNode
// if you change this method, you should change others
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
                mPassedNodeUUIDs);
            mPathCollection->add(path);
            info() << "build path: " << path.toString() << " with amount " << currentFlow;
        }
        return currentFlow;
    }
    if (level == mCurrentPathLength) {
        return 0;
    }

    auto trustLinePtrsSet =
            mTopologyTrustLineManager->trustLinePtrsSet(nodeUUID);
    if (trustLinePtrsSet.empty()) {
        return 0;
    }
    for (auto &trustLinePtr : trustLinePtrsSet) {
        auto trustLine = trustLinePtr->topologyTrustLine();
        if (trustLine->targetUUID() == mNodeUUID) {
            continue;
        }
        if (find(
                mPassedNodeUUIDs.begin(),
                mPassedNodeUUIDs.end(),
                trustLine->targetUUID()) != mPassedNodeUUIDs.end()) {
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
        mPassedNodeUUIDs.push_back(nodeUUID);
        TrustLineAmount calcFlow = calculateOneNode(
            trustLine->targetUUID(),
            nextFlow,
            level + (byte)1);
        mPassedNodeUUIDs.pop_back();
        if (calcFlow > TrustLine::kZeroAmount()) {
            trustLine->addUsedAmount(calcFlow);
            return calcFlow;
        }
    }
    return 0;
}

// this method used for rebuild paths in case of insufficient founds
// it used the same logic as PathsManager::buildPaths
// and InitiateMaxFlowCalculationTransaction::calculateMaxFlowOnOneLevel
void PathsManager::reBuildPaths(
    const NodeUUID &contractorUUID,
    const set<NodeUUID> &inaccessibleNodes)
{
    info() << "ReBuild paths to " << contractorUUID;
    auto startTime = utc_now();
    mTopologyTrustLineManager->makeFullyUsedTLsFromGatewaysToAllNodesExceptOne(
        contractorUUID);
    mContractorUUID = contractorUUID;
    mInaccessibleNodes = inaccessibleNodes;
    mPathCollection = make_shared<PathsCollection>(
        mNodeUUID,
        mContractorUUID);

    auto trustLinePtrsSet =
            mTopologyTrustLineManager->trustLinePtrsSet(mNodeUUID);
    if (trustLinePtrsSet.empty()) {
        mTopologyTrustLineManager->resetAllUsedAmounts();
        return;
    }

    // starts from 2, because direct path can't be rebuild
    for (mCurrentPathLength = 2; mCurrentPathLength <= kMaxPathLength; mCurrentPathLength++) {
        reBuildPathsOnOneLevel();
    }

    mTopologyTrustLineManager->resetAllUsedAmounts();
    info() << "rebuilding time: " << utc_now() - startTime;
}

// this method used for rebuild paths in case of insufficient founds
// it used the same logic as PathsManager::reBuildPathsOnOneLevel
// and InitiateMaxFlowCalculationTransaction::calculateMaxFlowOnOneLevel
TrustLineAmount PathsManager::reBuildPathsOnOneLevel()
{
    TrustLineAmount result = 0;
    auto trustLinePtrsSet =
            mTopologyTrustLineManager->trustLinePtrsSet(mNodeUUID);
    while(true) {
        TrustLineAmount currentFlow = 0;
        for (auto &trustLinePtr : trustLinePtrsSet) {
            auto trustLine = trustLinePtr->topologyTrustLine();
            auto trustLineFreeAmountShared = trustLine->freeAmount();
            auto trustLineAmountPtr = trustLineFreeAmountShared.get();
            mPassedNodeUUIDs.clear();
            if (mInaccessibleNodes.find(trustLine->targetUUID()) != mInaccessibleNodes.end()) {
                continue;
            }
            TrustLineAmount flow = calculateOneNodeForRebuildingPaths(
                trustLine->targetUUID(),
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

// this method used for rebuild paths in case of insufficient founds,
// it used the same logic as PathsManager::calculateOneNode
// and InitiateMaxFlowCalculationTransaction::calculateOneNode
// if you change this method, you should change others
TrustLineAmount PathsManager::calculateOneNodeForRebuildingPaths(
    const NodeUUID& nodeUUID,
    const TrustLineAmount& currentFlow,
    byte level)
{
    if (nodeUUID == mContractorUUID) {
        if (currentFlow > TrustLine::kZeroAmount()) {
            Path path(
                mNodeUUID,
                mContractorUUID,
                mPassedNodeUUIDs);
            mPathCollection->add(path);
            info() << "build path: " << path.toString() << " with amount " << currentFlow;
        }
        return currentFlow;
    }
    if (level == mCurrentPathLength) {
        return 0;
    }

    auto trustLinePtrsSet =
            mTopologyTrustLineManager->trustLinePtrsSet(nodeUUID);
    if (trustLinePtrsSet.empty()) {
        return 0;
    }
    for (auto &trustLinePtr : trustLinePtrsSet) {
        auto trustLine = trustLinePtr->topologyTrustLine();
        if (trustLine->targetUUID() == mNodeUUID) {
            continue;
        }

        if (mInaccessibleNodes.find(trustLine->targetUUID()) != mInaccessibleNodes.end()) {
            continue;
        }

        if (find(
                mPassedNodeUUIDs.begin(),
                mPassedNodeUUIDs.end(),
                trustLine->targetUUID()) != mPassedNodeUUIDs.end()) {
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
        mPassedNodeUUIDs.push_back(nodeUUID);
        TrustLineAmount calcFlow = calculateOneNodeForRebuildingPaths(
            trustLine->targetUUID(),
            nextFlow,
            level + (byte)1);
        mPassedNodeUUIDs.pop_back();
        if (calcFlow > TrustLine::kZeroAmount()) {
            trustLine->addUsedAmount(calcFlow);
            return calcFlow;
        }
    }
    return 0;
}

void PathsManager::addUsedAmount(
    const NodeUUID &sourceUUID,
    const NodeUUID &targetUUID,
    const TrustLineAmount &amount)
{
    mTopologyTrustLineManager->addUsedAmount(
        sourceUUID,
        targetUUID,
        amount);
}

void PathsManager::makeTrustLineFullyUsed(
    const NodeUUID &sourceUUID,
    const NodeUUID &targetUUID)
{
    mTopologyTrustLineManager->makeFullyUsed(
        sourceUUID,
        targetUUID);
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
    stringstream s;
    s << "[PathsManager: " << mEquivalent << "] ";
    return s.str();
}