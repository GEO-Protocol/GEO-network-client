#include "InitiateMaxFlowCalculationTransaction.h"

InitiateMaxFlowCalculationTransaction::InitiateMaxFlowCalculationTransaction(
    NodeUUID &nodeUUID,
    InitiateMaxFlowCalculationCommand::Shared command,
    TrustLinesManager *trustLinesManager,
    TopologyTrustLinesManager *topologyTrustLineManager,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    bool iAmGateway,
    Logger &logger) :

    BaseCollectTopologyTransaction(
        BaseTransaction::InitiateMaxFlowCalculationTransactionType,
        nodeUUID,
        command->equivalent(),
        trustLinesManager,
        topologyTrustLineManager,
        topologyCacheManager,
        maxFlowCacheManager,
        logger),
    mCommand(command),
    mIAmGateway(iAmGateway)
{}

InitiateMaxFlowCalculationCommand::Shared InitiateMaxFlowCalculationTransaction::command() const
{
    return mCommand;
}

TransactionResult::SharedConst InitiateMaxFlowCalculationTransaction::sendRequestForCollectingTopology()
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
            maxFlows.emplace_back(
                contractorUUID,
                TrustLineAmount(0));
        }
        return resultOk(true, maxFlows);
    }

    vector<NodeUUID> nonCachedContractors;
    bool allContractorsAlreadyCalculated = true;
    for (const auto &contractor : mCommand->contractors()) {
        auto nodeCache = mMaxFlowCacheManager->cacheByNode(contractor);
        if (nodeCache == nullptr) {
            nonCachedContractors.push_back(contractor);
            allContractorsAlreadyCalculated = false;
        } else {
            allContractorsAlreadyCalculated &= nodeCache->isFlowFinal();
        }
    }

    if (allContractorsAlreadyCalculated) {
        vector<pair<NodeUUID, TrustLineAmount>> maxFlows;
        maxFlows.reserve(mCommand->contractors().size());
        for (const auto &contractorUUID : mCommand->contractors()) {
            auto nodeCache = mMaxFlowCacheManager->cacheByNode(contractorUUID);
                maxFlows.emplace_back(
                    contractorUUID,
                    nodeCache->currentFlow());
        }
        return resultOk(true, maxFlows);
    }

    const auto kTransaction = make_shared<CollectTopologyTransaction>(
        mNodeUUID,
        mEquivalent,
        nonCachedContractors,
        mTrustLinesManager,
        mTopologyTrustLineManager,
        mTopologyCacheManager,
        mMaxFlowCacheManager,
        mLog);
    mTopologyTrustLineManager->setPreventDeleting(true);
    launchSubsidiaryTransaction(kTransaction);
    return resultAwakeAfterMilliseconds(
        kWaitMillisecondsForCalculatingMaxFlow);
}

TransactionResult::SharedConst InitiateMaxFlowCalculationTransaction::processCollectingTopology()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "CalculateMaxTransactionFlow";
    info() << "context size: " << mContext.size();
#endif
    fillTopology();
    vector<pair<NodeUUID, TrustLineAmount>> maxFlows;
    maxFlows.reserve(mCommand->contractors().size());
    mFirstLevelTopology =
            mTopologyTrustLineManager->trustLinePtrsSet(mNodeUUID);
    auto startTime = utc_now();
    for (const auto &contractorUUID : mCommand->contractors()) {
        auto nodeCache = mMaxFlowCacheManager->cacheByNode(contractorUUID);
        if (nodeCache != nullptr) {
            maxFlows.emplace_back(
                contractorUUID,
                nodeCache->currentFlow());
        } else {
            maxFlows.emplace_back(
                contractorUUID,
                // todo : choose updated or not
                calculateMaxFlow(
                    contractorUUID));
        }
    }
    info() << "all contractors calculating time: " << utc_now() - startTime;
    const auto kTransaction = make_shared<MaxFlowCalculationStepTwoTransaction>(
        mNodeUUID,
        currentTransactionUUID(),
        mCommand,
        mTrustLinesManager,
        mTopologyTrustLineManager,
        mTopologyCacheManager,
        mMaxFlowCacheManager,
        2,
        mLog);
    launchSubsidiaryTransaction(kTransaction);
    return resultOk(false, maxFlows);
}

// this method used the same logic as PathsManager::reBuildPaths
// and PathsManager::buildPaths
TrustLineAmount InitiateMaxFlowCalculationTransaction::calculateMaxFlow(
    const NodeUUID &contractorUUID)
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "start found flow to: " << contractorUUID;
    info() << "gateways:";
    for (auto const &gateway : mTopologyTrustLineManager->gateways()) {
        info() << "\t" << gateway;
    }
#endif
    DateTime startTime = utc_now();

    mTopologyTrustLineManager->makeFullyUsedTLsFromGatewaysToAllNodesExceptOne(
        contractorUUID);
    mCurrentContractor = contractorUUID;
    if (mFirstLevelTopology.empty()) {
        mTopologyTrustLineManager->resetAllUsedAmounts();
        return TrustLine::kZeroAmount();
    }

    mCurrentMaxFlow = TrustLine::kZeroAmount();
    for (mCurrentPathLength = 1; mCurrentPathLength <= kMaxPathLength; mCurrentPathLength++) {
        calculateMaxFlowOnOneLevel();
    }

    mTopologyTrustLineManager->resetAllUsedAmounts();
    info() << "max flow calculating time: " << utc_now() - startTime;
    mMaxFlowCacheManager->addCache(
        mCurrentContractor,
        make_shared<MaxFlowCache>(
            mCurrentMaxFlow));
    return mCurrentMaxFlow;
}

TrustLineAmount InitiateMaxFlowCalculationTransaction::calculateMaxFlowUpdated(
    const NodeUUID &contractorUUID)
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "start found flow to: " << contractorUUID;
    info() << "gateways:";
    for (auto const &gateway : mTopologyTrustLineManager->gateways()) {
        info() << "\t" << gateway;
    }
#endif
    DateTime startTime = utc_now();

    mTopologyTrustLineManager->makeFullyUsedTLsFromGatewaysToAllNodesExceptOne(
            contractorUUID);
    mCurrentContractor = contractorUUID;
    auto trustLinePtrsSet =
            mTopologyTrustLineManager->trustLinePtrsSet(mNodeUUID);
    if (trustLinePtrsSet.empty()) {
        mTopologyTrustLineManager->resetAllUsedAmounts();
        return TrustLine::kZeroAmount();
    }

    mCurrentMaxFlow = TrustLine::kZeroAmount();
    for (mCurrentPathLength = 1; mCurrentPathLength <= kMaxPathLength; mCurrentPathLength++) {
        calculateMaxFlowOnOneLevelUpdated();
    }

    mTopologyTrustLineManager->resetAllUsedAmounts();
    info() << "max flow updated calculating time: " << utc_now() - startTime;
    return mCurrentMaxFlow;
}

// this method used the same logic as PathsManager::reBuildPathsOnOneLevel
// and PathsManager::buildPathsOnOneLevel
void InitiateMaxFlowCalculationTransaction::calculateMaxFlowOnOneLevel()
{
    while(true) {
        TrustLineAmount currentFlow = 0;
        for (auto &trustLinePtr : mFirstLevelTopology) {
            auto trustLine = trustLinePtr->topologyTrustLine();
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

void InitiateMaxFlowCalculationTransaction::calculateMaxFlowOnOneLevelUpdated()
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
        mForbiddenNodeUUIDs.clear();
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
// and PathsManager::calculateOneNode
// if you change this method, you should change others
TrustLineAmount InitiateMaxFlowCalculationTransaction::calculateOneNode(
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

TransactionResult::SharedConst InitiateMaxFlowCalculationTransaction::resultOk(
    bool finalMaxFlows,
    vector<pair<NodeUUID, TrustLineAmount>> &maxFlows)
{
    stringstream ss;
    if (finalMaxFlows) {
        ss << kFinalStep << kTokensSeparator << maxFlows.size();
    } else {
        ss << "1" << kTokensSeparator << maxFlows.size();
    }
    for (const auto &nodeUUIDAndMaxFlow : maxFlows) {
        ss << kTokensSeparator << nodeUUIDAndMaxFlow.first << kTokensSeparator;
        ss << nodeUUIDAndMaxFlow.second;
    }
    auto kMaxFlowAmountsStr = ss.str();
    return transactionResultFromCommand(
        mCommand->responseOk(
            kMaxFlowAmountsStr));
}

TransactionResult::SharedConst InitiateMaxFlowCalculationTransaction::resultProtocolError()
{
    return transactionResultFromCommand(
        mCommand->responseProtocolError());
}

const string InitiateMaxFlowCalculationTransaction::logHeader() const
{
    stringstream s;
    s << "[InitiateMaxFlowCalculationTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}