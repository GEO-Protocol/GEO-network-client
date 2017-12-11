#include "MaxFlowCalculationFullyTransaction.h"

MaxFlowCalculationFullyTransaction::MaxFlowCalculationFullyTransaction(
    const NodeUUID &nodeUUID,
    const TransactionUUID &transactionUUID,
    const InitiateMaxFlowCalculationCommand::Shared command,
    TrustLinesManager *trustLinesManager,
    MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    MaxFlowCalculationNodeCacheManager *maxFlowCalculationNodeCacheManager,
    Logger &logger) :

    BaseCollectTopologyTransaction(
        BaseTransaction::MaxFlowCalculationFullyTransactionType,
        transactionUUID,
        nodeUUID,
        trustLinesManager,
        maxFlowCalculationTrustLineManager,
        maxFlowCalculationCacheManager,
        maxFlowCalculationNodeCacheManager,
        logger),
    mCommand(command)
{}

InitiateMaxFlowCalculationCommand::Shared MaxFlowCalculationFullyTransaction::command() const
{
    return mCommand;
}

TransactionResult::SharedConst MaxFlowCalculationFullyTransaction::sendRequestForCollectingTopology()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "run\t" << "initiator: " << mNodeUUID;
    info() << "run\t" << "targets count: " << mCommand->contractors().size();
    info() << "SendRequestForCollectingTopology";
#endif
    mCountProcessCollectingTopologyRun = 0;
    return resultAwaikAfterMilliseconds(
        kWaitMillisecondsForCalculatingMaxFlow);
}

TransactionResult::SharedConst MaxFlowCalculationFullyTransaction::processCollectingTopology()
{
//#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "CalculateMaxTransactionFlow";
    info() << "context size: " << mContext.size();
//#endif
    auto const contextSize = mContext.size();
    fillTopology();
    mCountProcessCollectingTopologyRun++;
    if (contextSize > 0 && mCountProcessCollectingTopologyRun <= kCountRunningProcessCollectingTopologyStage) {
        return resultAwaikAfterMilliseconds(
            kWaitMillisecondsForCalculatingMaxFlowAgain);
    }
    vector<pair<NodeUUID, TrustLineAmount>> maxFlows;
    maxFlows.reserve(mCommand->contractors().size());
    mFirstLevelTopology =
            mMaxFlowCalculationTrustLineManager->trustLinePtrsSet(mNodeUUID);
    auto startTime = utc_now();
    for (const auto &contractorUUID : mCommand->contractors()) {
        auto nodeCache = mMaxFlowCalculationNodeCacheManager->cacheByNode(contractorUUID);
        // todo : implement separated logic when !nodeCache->isFlowFinal()
        if (nodeCache != nullptr && nodeCache->isFlowFinal()) {
            maxFlows.push_back(
                make_pair(
                    contractorUUID,
                    nodeCache->currentFlow()));
        } else {
            maxFlows.push_back(
                make_pair(
                    contractorUUID,
                    calculateMaxFlow(
                        contractorUUID)));
        }
    }
    info() << "all contractors calculating time: " << utc_now() - startTime;
    mMaxFlowCalculationTrustLineManager->setPreventDeleting(false);
    return resultOk(maxFlows);
}

// this method used the same logic as PathsManager::reBuildPaths
// and PathsManager::buildPaths
TrustLineAmount MaxFlowCalculationFullyTransaction::calculateMaxFlow(
    const NodeUUID &contractorUUID)
{
//#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "calculateMaxFlow\tstart found flow to: " << contractorUUID;
    DateTime startTime = utc_now();
//#endif
    mCurrentContractor = contractorUUID;
    if (mFirstLevelTopology.size() == 0) {
        mMaxFlowCalculationTrustLineManager->resetAllUsedAmounts();
        return TrustLine::kZeroAmount();
    }

    mCurrentMaxFlow = TrustLine::kZeroAmount();
    for (mCurrentPathLength = 1; mCurrentPathLength <= kMaxPathLength; mCurrentPathLength++) {
        calculateMaxFlowOnOneLevel();
    }

    mMaxFlowCalculationTrustLineManager->resetAllUsedAmounts();
//#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "max flow calculating time: " << utc_now() - startTime;
//#endif
    mMaxFlowCalculationNodeCacheManager->updateCache(
        mCurrentContractor,
        mCurrentMaxFlow,
        true);
    return mCurrentMaxFlow;
}

// this method used the same logic as PathsManager::reBuildPathsOnOneLevel
// and PathsManager::buildPathsOnOneLevel
void MaxFlowCalculationFullyTransaction::calculateMaxFlowOnOneLevel()
{
    while(true) {
        TrustLineAmount currentFlow = 0;
        for (auto &trustLinePtr : mFirstLevelTopology) {
            auto trustLine = trustLinePtr->maxFlowCalculationtrustLine();
            auto trustLineFreeAmountShared = trustLine->freeAmount();
            auto trustLineAmountPtr = trustLineFreeAmountShared.get();
            mForbiddenNodeUUIDs.clear();
            TrustLineAmount flow = calculateOneNode(
                trustLine->targetUUID(),
                *trustLineAmountPtr,
                1);
            if (flow > TrustLine::kZeroAmount()) {
                currentFlow += flow;
                trustLine->addUsedAmount(flow);
            }
        }
        if (currentFlow == 0) {
            break;
        }
    }
}

// it used the same logic as PathsManager::calculateOneNodeForRebuildingPaths
// and PathsManager::calculateOneNode
// if you change this method, you should change others
TrustLineAmount MaxFlowCalculationFullyTransaction::calculateOneNode(
    const NodeUUID& nodeUUID,
    const TrustLineAmount& currentFlow,
    byte level)
{
    if (nodeUUID == mCurrentContractor) {
        if (currentFlow > TrustLine::kZeroAmount()) {
            mCurrentMaxFlow += currentFlow;
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
        if (trustLine->targetUUID() == mNodeUUID) {
            continue;
        }
        if (find(
                mForbiddenNodeUUIDs.begin(),
                mForbiddenNodeUUIDs.end(),
                trustLine->targetUUID()) != mForbiddenNodeUUIDs.end()) {
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
        mForbiddenNodeUUIDs.push_back(nodeUUID);
        TrustLineAmount calcFlow = calculateOneNode(
            trustLine->targetUUID(),
            nextFlow,
            level + (byte)1);
        mForbiddenNodeUUIDs.pop_back();
        if (calcFlow > TrustLine::kZeroAmount()) {
            trustLine->addUsedAmount(calcFlow);
            return calcFlow;
        }
    }
    return 0;
}

TransactionResult::SharedConst MaxFlowCalculationFullyTransaction::resultOk(
    vector<pair<NodeUUID, TrustLineAmount>> &maxFlows)
{
    stringstream ss;
    ss << "2" << "\t" << maxFlows.size();
    for (const auto &nodeUUIDAndMaxFlow : maxFlows) {
        ss << "\t" << nodeUUIDAndMaxFlow.first << "\t";
        ss << nodeUUIDAndMaxFlow.second;
    }
    auto kMaxFlowAmountsStr = ss.str();
    return transactionResultFromCommand(
        mCommand->responseOk(
            kMaxFlowAmountsStr));
}

const string MaxFlowCalculationFullyTransaction::logHeader() const
{
    stringstream s;
    s << "[MaxFlowCalculationFullyTA: " << currentTransactionUUID() << "]";
    return s.str();
}