#include "PathsManager.h"

PathsManager::PathsManager(
    const SerializedEquivalent equivalent,
    TrustLinesManager *trustLinesManager,
    TopologyTrustLinesManager *topologyTrustLineManager,
    Logger &logger):

    mEquivalent(equivalent),
    mTrustLinesManager(trustLinesManager),
    mTopologyTrustLineManager(topologyTrustLineManager),
    mLog(logger),
    mPathCollection(nullptr)
{}

bool PathsManager::isPathValid(const Path &path)
{
    auto itGlobal = path.intermediates().begin();
    while (itGlobal != path.intermediates().end() - 1) {
        auto itLocal = itGlobal + 1;
        while (itLocal != path.intermediates().end()) {
            if (*itGlobal == *itLocal) {
                return false;
            }
            itLocal++;
        }
        itGlobal++;
    }
    return true;
}

// this method used the same logic as InitiateMaxFlowCalculationTransaction::calculateMaxFlow
void PathsManager::buildPaths(
    BaseAddress::Shared contractorAddress,
    ContractorID contractorID)
{
    info() << "Build paths to " << contractorAddress->fullAddress() << " id " << contractorID;
    auto startTime = utc_now();
    mTopologyTrustLineManager->makeFullyUsedTLsFromGatewaysToAllNodesExceptOneNew(
        contractorID);
    mContractorID = contractorID;
    mPathCollection = make_shared<PathsCollection>(
        contractorAddress);
    auto trustLinePtrsSet =
            mTopologyTrustLineManager->trustLinePtrsSetNew(0);
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
            mTopologyTrustLineManager->trustLinePtrsSetNew(0);
    auto itTrustLinePtr = trustLinePtrsSet.begin();
    while (itTrustLinePtr != trustLinePtrsSet.end()) {
        auto trustLine = (*itTrustLinePtr)->topologyTrustLine();
        auto trustLineFreeAmountShared = trustLine->freeAmount();
        auto trustLineAmountPtr = trustLineFreeAmountShared.get();
        if (*trustLineAmountPtr == TrustLine::kZeroAmount()) {
            itTrustLinePtr++;
            continue;
        }
        mPassedNodeIDs.clear();
        TrustLineAmount flow = calculateOneNode(
            trustLine->targetID(),
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
            mTopologyTrustLineManager->trustLinePtrsSetNew(0);
    auto gateways = mTrustLinesManager->gatewaysNew();
    while (!gateways.empty()) {
        auto itGateway = gateways.begin();
        for (auto itTrustLinePtr = trustLinePtrsSet.begin(); itTrustLinePtr != trustLinePtrsSet.end(); itTrustLinePtr++) {
            auto trustLine = (*itTrustLinePtr)->topologyTrustLine();
            bool isContinue = true;
            if (trustLine->targetID() == *itGateway) {
                auto trustLineFreeAmountShared = trustLine->freeAmount();
                auto trustLineAmountPtr = trustLineFreeAmountShared.get();
                mPassedNodeIDs.clear();
                TrustLineAmount flow = calculateOneNode(
                    trustLine->targetID(),
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
        mPassedNodeIDs.clear();
        TrustLineAmount flow = calculateOneNode(
            trustLine->targetID(),
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
    ContractorID nodeID,
    const TrustLineAmount &currentFlow,
    byte level)
{
    if (nodeID == mContractorID) {
        if (currentFlow > TrustLine::kZeroAmount()) {
            auto pathWithAddresses = addressesPath();
            auto path = make_shared<Path>(pathWithAddresses);
            mPathCollection->add(path);
            info() << "build path: " << path->toString() << " with amount " << currentFlow;
        }
        return currentFlow;
    }
    if (level == mCurrentPathLength) {
        return 0;
    }

    auto trustLinePtrsSet =
        mTopologyTrustLineManager->trustLinePtrsSetNew(nodeID);
    if (trustLinePtrsSet.empty()) {
        return 0;
    }
    for (auto &trustLinePtr : trustLinePtrsSet) {
        auto trustLine = trustLinePtr->topologyTrustLine();
        if (trustLine->targetID() == 0) {
            continue;
        }
        if (find(
                mPassedNodeIDs.begin(),
                mPassedNodeIDs.end(),
                trustLine->targetID()) != mPassedNodeIDs.end()) {
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
        mPassedNodeIDs.push_back(nodeID);
        TrustLineAmount calcFlow = calculateOneNode(
            trustLine->targetID(),
            nextFlow,
            level + (byte) 1);
        mPassedNodeIDs.pop_back();
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
    BaseAddress::Shared contractorAddress,
    const vector<BaseAddress::Shared> &inaccessibleNodes)
{
    mContractorAddress = contractorAddress;
    mContractorID = mTopologyTrustLineManager->getID(contractorAddress);
    info() << "ReBuild paths to " << mContractorAddress->fullAddress() << " id " << mContractorID;
    auto startTime = utc_now();
    mTopologyTrustLineManager->makeFullyUsedTLsFromGatewaysToAllNodesExceptOneNew(
        mContractorID);
    for (const auto &inaccessibleNodeAddress : inaccessibleNodes) {
        mInaccessibleNodes.insert(
            mTopologyTrustLineManager->getID(
                inaccessibleNodeAddress));
    }
    mPathCollection = make_shared<PathsCollection>(
        contractorAddress);

    auto trustLinePtrsSet =
            mTopologyTrustLineManager->trustLinePtrsSetNew(0);
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
            mTopologyTrustLineManager->trustLinePtrsSetNew(0);
    while(true) {
        TrustLineAmount currentFlow = 0;
        for (auto &trustLinePtr : trustLinePtrsSet) {
            auto trustLine = trustLinePtr->topologyTrustLine();
            auto trustLineFreeAmountShared = trustLine->freeAmount();
            auto trustLineAmountPtr = trustLineFreeAmountShared.get();
            mPassedNodeIDs.clear();
            if (mInaccessibleNodes.find(trustLine->targetID()) != mInaccessibleNodes.end()) {
                continue;
            }
            TrustLineAmount flow = calculateOneNodeForRebuildingPaths(
                trustLine->targetID(),
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
    ContractorID nodeID,
    const TrustLineAmount& currentFlow,
    byte level)
{
    if (nodeID == mContractorID) {
        if (currentFlow > TrustLine::kZeroAmount()) {
            auto pathWithAddresses = addressesPath();
            auto path = make_shared<Path>(pathWithAddresses);
            mPathCollection->add(path);
            info() << "build path: " << path->toString() << " with amount " << currentFlow;
        }
        return currentFlow;
    }
    if (level == mCurrentPathLength) {
        return 0;
    }

    auto trustLinePtrsSet =
            mTopologyTrustLineManager->trustLinePtrsSetNew(nodeID);
    if (trustLinePtrsSet.empty()) {
        return 0;
    }
    for (auto &trustLinePtr : trustLinePtrsSet) {
        auto trustLine = trustLinePtr->topologyTrustLine();
        if (trustLine->targetID() == 0) {
            continue;
        }

        if (mInaccessibleNodes.find(trustLine->targetID()) != mInaccessibleNodes.end()) {
            continue;
        }

        if (find(
                mPassedNodeIDs.begin(),
                mPassedNodeIDs.end(),
                trustLine->targetID()) != mPassedNodeIDs.end()) {
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
        mPassedNodeIDs.push_back(nodeID);
        TrustLineAmount calcFlow = calculateOneNodeForRebuildingPaths(
            trustLine->targetID(),
            nextFlow,
            level + (byte)1);
        mPassedNodeIDs.pop_back();
        if (calcFlow > TrustLine::kZeroAmount()) {
            trustLine->addUsedAmount(calcFlow);
            return calcFlow;
        }
    }
    return 0;
}

void PathsManager::addUsedAmount(
    BaseAddress::Shared sourceAddress,
    BaseAddress::Shared targetAddress,
    const TrustLineAmount &amount)
{
    auto sourceID = mTopologyTrustLineManager->getID(sourceAddress);
    auto targetID = mTopologyTrustLineManager->getID(targetAddress);
    mTopologyTrustLineManager->addUsedAmountNew(
        sourceID,
        targetID,
        amount);
}

void PathsManager::makeTrustLineFullyUsed(
    BaseAddress::Shared sourceAddress,
    BaseAddress::Shared targetAddress)
{
    auto sourceID = mTopologyTrustLineManager->getID(sourceAddress);
    auto targetID = mTopologyTrustLineManager->getID(targetAddress);
    mTopologyTrustLineManager->makeFullyUsedNew(
        sourceID,
        targetID);
}

vector<BaseAddress::Shared> PathsManager::addressesPath()
{
    vector<BaseAddress::Shared> result;
    result.reserve(mPassedNodeIDs.size());
    for (const auto &nodeID : mPassedNodeIDs) {
        result.push_back(
            mTopologyTrustLineManager->getAddressByID(
                nodeID));
    }
    return result;
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