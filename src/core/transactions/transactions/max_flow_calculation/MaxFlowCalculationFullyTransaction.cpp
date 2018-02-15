#include "MaxFlowCalculationFullyTransaction.h"

MaxFlowCalculationFullyTransaction::MaxFlowCalculationFullyTransaction(
    const NodeUUID &nodeUUID,
    const InitiateMaxFlowCalculationFullyCommand::Shared command,
    TrustLinesManager *trustLinesManager,
    MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    MaxFlowCalculationNodeCacheManager *maxFlowCalculationNodeCacheManager,
    Logger &logger) :

    BaseCollectTopologyTransaction(
        BaseTransaction::MaxFlowCalculationFullyTransactionType,
        nodeUUID,
        trustLinesManager,
        maxFlowCalculationTrustLineManager,
        maxFlowCalculationCacheManager,
        maxFlowCalculationNodeCacheManager,
        logger),
    mCommand(command)
{}

InitiateMaxFlowCalculationFullyCommand::Shared MaxFlowCalculationFullyTransaction::command() const
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
    // Check if there is mNodeUUID in command parameters
    for (const auto &contractorUUID : mCommand->contractors()) {
        if (contractorUUID == currentNodeUUID()) {
            warning() << "Attempt to initialise operation against itself was prevented. Canceled.";
            return resultProtocolError();
        }
    }
    // Check if Node does not have outgoing FlowAmount;
    if(mTrustLinesManager->firstLevelNeighborsWithOutgoingFlow().empty()){
        vector<pair<NodeUUID, TrustLineAmount>> maxFlows;
        maxFlows.reserve(mCommand->contractors().size());
        for (const auto &contractorUUID : mCommand->contractors()) {
            maxFlows.push_back(
                make_pair(
                    contractorUUID,
                    TrustLineAmount(0)));
        }
        return resultOk(maxFlows);
    }

    const auto kTransaction = make_shared<CollectTopologyTransaction>(
        mNodeUUID,
        mCommand->contractors(),
        mTrustLinesManager,
        mMaxFlowCalculationTrustLineManager,
        mMaxFlowCalculationCacheManager,
        mMaxFlowCalculationNodeCacheManager,
        mLog);
    mMaxFlowCalculationTrustLineManager->setPreventDeleting(true);
    launchSubsidiaryTransaction(kTransaction);

    mCountProcessCollectingTopologyRun = 0;
    return resultAwakeAfterMilliseconds(
        kWaitMillisecondsForCalculatingMaxFlow);
}

TransactionResult::SharedConst MaxFlowCalculationFullyTransaction::processCollectingTopology()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "CalculateMaxTransactionFlow";
    info() << "context size: " << mContext.size();
#endif
    auto const contextSize = mContext.size();
    fillTopology();
    mCountProcessCollectingTopologyRun++;
    if (contextSize > 0 && mCountProcessCollectingTopologyRun <= kCountRunningProcessCollectingTopologyStage) {
        return resultAwakeAfterMilliseconds(
            kWaitMillisecondsForCalculatingMaxFlowAgain);
    }
    vector<pair<NodeUUID, TrustLineAmount>> maxFlows;
    maxFlows.reserve(mCommand->contractors().size());
    mFirstLevelTopology =
            mMaxFlowCalculationTrustLineManager->trustLinePtrsSet(mNodeUUID);
    auto startTime = utc_now();
    for (const auto &contractorUUID : mCommand->contractors()) {
        maxFlows.push_back(
            make_pair(
                contractorUUID,
                calculateMaxFlow(
                    contractorUUID)));
    }
    info() << "all contractors calculating time: " << (utc_now() - startTime);
    mMaxFlowCalculationTrustLineManager->setPreventDeleting(false);
    return resultOk(maxFlows);
}

// this method used the same logic as PathsManager::reBuildPaths
// and PathsManager::buildPaths
TrustLineAmount MaxFlowCalculationFullyTransaction::calculateMaxFlow(
    const NodeUUID &contractorUUID)
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "calculateMaxFlow\tstart found flow to: " << contractorUUID;
#endif
    DateTime startTime = utc_now();

    mMaxFlowCalculationTrustLineManager->makeFullyUsedTLsFromGatewaysToAllNodesExceptOne(
        contractorUUID);

    mCurrentContractor = contractorUUID;
    if (mFirstLevelTopology.empty()) {
        mMaxFlowCalculationTrustLineManager->resetAllUsedAmounts();
        return TrustLine::kZeroAmount();
    }

    mCurrentMaxFlow = TrustLine::kZeroAmount();
    for (mCurrentPathLength = 1; mCurrentPathLength <= kMaxPathLength; mCurrentPathLength++) {
        calculateMaxFlowOnOneLevel();
    }

    mMaxFlowCalculationTrustLineManager->resetAllUsedAmounts();
    info() << "max flow calculating time: " << utc_now() - startTime;

    auto nodeCache = mMaxFlowCalculationNodeCacheManager->cacheByNode(mCurrentContractor);
    if (nodeCache != nullptr) {
        mMaxFlowCalculationNodeCacheManager->updateCache(
            mCurrentContractor,
            mCurrentMaxFlow,
            true);
    } else {
        mMaxFlowCalculationNodeCacheManager->addCache(
            mCurrentContractor,
            make_shared<MaxFlowCalculationNodeCache>(
                mCurrentMaxFlow,
                true));
    }

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
    if (trustLinePtrsSet.empty()) {
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
    ss << maxFlows.size();
    for (const auto &nodeUUIDAndMaxFlow : maxFlows) {
        ss << "\t" << nodeUUIDAndMaxFlow.first << "\t";
        ss << nodeUUIDAndMaxFlow.second;
    }
    auto kMaxFlowAmountsStr = ss.str();
    return transactionResultFromCommand(
        mCommand->responseOk(
            kMaxFlowAmountsStr));
}

TransactionResult::SharedConst MaxFlowCalculationFullyTransaction::resultProtocolError()
{
    return transactionResultFromCommand(
        mCommand->responseProtocolError());
}

const string MaxFlowCalculationFullyTransaction::logHeader() const
{
    stringstream s;
    s << "[MaxFlowCalculationFullyTA: " << currentTransactionUUID() << "]";
    return s.str();
}