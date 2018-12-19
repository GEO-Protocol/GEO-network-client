#include "MaxFlowCalculationFullyTransaction.h"

MaxFlowCalculationFullyTransaction::MaxFlowCalculationFullyTransaction(
    const NodeUUID &nodeUUID,
    const InitiateMaxFlowCalculationFullyCommand::Shared command,
    ContractorsManager *contractorsManager,
    TrustLinesManager *trustLinesManager,
    TopologyTrustLinesManager *topologyTrustLineManager,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    bool iAmGateway,
    Logger &logger) :

    BaseCollectTopologyTransaction(
        BaseTransaction::MaxFlowCalculationFullyTransactionType,
        nodeUUID,
        command->equivalent(),
        contractorsManager,
        trustLinesManager,
        topologyTrustLineManager,
        topologyCacheManager,
        maxFlowCacheManager,
        logger),
    mCommand(command),
    mIamGateway(iAmGateway)
{}

TransactionResult::SharedConst MaxFlowCalculationFullyTransaction::sendRequestForCollectingTopology()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "run\t" << "initiator: " << mNodeUUID;
    info() << "run\t" << "targets count: " << mCommand->contractors().size();
    info() << "SendRequestForCollectingTopology";
#endif
    info() << "contractors:";
    for (const auto &contractor : mCommand->contractors()) {
        info() << contractor;
    }
    info() << "addresses:";
    for (const auto &contractor : mCommand->contractorAddresses()) {
        info() << contractor->fullAddress();
    }
    // Check if there is mNodeUUID in command parameters
    // todo : check if contractorAddresses don't include ownAddress
    for (const auto &contractorUUID : mCommand->contractors()) {
        if (contractorUUID == currentNodeUUID()) {
            warning() << "Attempt to initialise operation against itself was prevented. Canceled.";
            return resultProtocolError();
        }
    }
    for (const auto &contractorAddress : mCommand->contractorAddresses()) {
        auto contractorID = mTopologyTrustLineManager->getID(
            contractorAddress);
        info() << "ContractorID " << contractorID << " address " << contractorAddress->fullAddress();
        mContractorIDs.emplace_back(
            contractorID,
            contractorAddress);
    }
    // Check if Node does not have outgoing FlowAmount;
    if(mTrustLinesManager->firstLevelNeighborsWithOutgoingFlow().empty()){
        mMaxFlows.reserve(mCommand->contractors().size());
        for (const auto &contractorUUID : mCommand->contractors()) {
            mMaxFlows.emplace_back(
                contractorUUID,
                TrustLineAmount(0));
        }
        for (const auto &contractorIDAndAddress : mContractorIDs) {
            mMaxFlowsNew.emplace_back(
                contractorIDAndAddress.first,
                TrustLineAmount(0));
        }
        return resultOk();
    }

    const auto kTransaction = make_shared<CollectTopologyTransaction>(
        mNodeUUID,
        mEquivalent,
        mCommand->contractorAddresses(),
        mContractorsManager,
        mTrustLinesManager,
        mTopologyTrustLineManager,
        mTopologyCacheManager,
        mMaxFlowCacheManager,
        mIamGateway,
        mLog);
    mTopologyTrustLineManager->setPreventDeleting(true);
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
    mMaxFlows.reserve(mCommand->contractors().size());
    mMaxFlows.reserve(mContractorIDs.size());
    mFirstLevelTopology =
            mTopologyTrustLineManager->trustLinePtrsSet(mNodeUUID);
    mFirstLevelTopologyNew =
            mTopologyTrustLineManager->trustLinePtrsSetNew(0);
    mCurrentGlobalContractorIdx = 0;
    mStep = CustomLogic;
    return applyCustomLogic();
}

TransactionResult::SharedConst MaxFlowCalculationFullyTransaction::applyCustomLogic()
{
    if (mCurrentGlobalContractorIdx >= mCommand->contractors().size()) {
        error() << "Illegal current contractor idx: " << mCurrentGlobalContractorIdx;
        mTopologyTrustLineManager->setPreventDeleting(false);
        return resultOk();
    }
    const auto contractorUUID = mCommand->contractors().at(mCurrentGlobalContractorIdx);
    mMaxFlows.emplace_back(
        contractorUUID,
        calculateMaxFlow(
            contractorUUID));
    const auto contractorIDAndAddress = mContractorIDs.at(mCurrentGlobalContractorIdx);
    auto maxFlow = calculateMaxFlowNew(
        contractorIDAndAddress.first);
    mMaxFlowsNew.emplace_back(
        contractorIDAndAddress.first,
        maxFlow);
    auto nodeCacheNew = mMaxFlowCacheManager->cacheByAddress(contractorIDAndAddress.second);
    if (nodeCacheNew != nullptr) {
        mMaxFlowCacheManager->updateCacheNew(
            contractorIDAndAddress.second,
            maxFlow,
            true);
    } else {
        mMaxFlowCacheManager->addCacheNew(
            contractorIDAndAddress.second,
            make_shared<MaxFlowCache>(
                maxFlow,
                true));
    }
    if (mMaxFlows.size() >= mCommand->contractors().size()) {
        mTopologyTrustLineManager->setPreventDeleting(false);
        return resultOk();
    }
    mCurrentGlobalContractorIdx++;
    return resultAwakeAsFastAsPossible();
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

    auto nodeCache = mMaxFlowCacheManager->cacheByNode(mCurrentContractor);
    if (nodeCache != nullptr) {
        mMaxFlowCacheManager->updateCache(
            mCurrentContractor,
            mCurrentMaxFlow,
            true);
    } else {
        mMaxFlowCacheManager->addCache(
            mCurrentContractor,
            make_shared<MaxFlowCache>(
                mCurrentMaxFlow,
                true));
    }

    return mCurrentMaxFlow;
}

TrustLineAmount MaxFlowCalculationFullyTransaction::calculateMaxFlowNew(
    ContractorID contractorID)
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "calculateMaxFlowNew\tstart found flow to: " << contractorID;
#endif
    DateTime startTime = utc_now();

    mTopologyTrustLineManager->makeFullyUsedTLsFromGatewaysToAllNodesExceptOneNew(
        contractorID);

    mCurrentContractorNew = contractorID;
    if (mFirstLevelTopologyNew.empty()) {
        mTopologyTrustLineManager->resetAllUsedAmounts();
        return TrustLine::kZeroAmount();
    }

    mCurrentMaxFlow = TrustLine::kZeroAmount();
    for (mCurrentPathLength = 1; mCurrentPathLength <= kMaxPathLength; mCurrentPathLength++) {
        calculateMaxFlowOnOneLevelNew();
    }

    mTopologyTrustLineManager->resetAllUsedAmounts();
    info() << "max flow calculating time: " << utc_now() - startTime;

    return mCurrentMaxFlow;
}

// this method used the same logic as PathsManager::reBuildPathsOnOneLevel
// and PathsManager::buildPathsOnOneLevel
void MaxFlowCalculationFullyTransaction::calculateMaxFlowOnOneLevel()
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

void MaxFlowCalculationFullyTransaction::calculateMaxFlowOnOneLevelNew()
{
    while(true) {
        TrustLineAmount currentFlow = 0;
        for (auto &trustLinePtr : mFirstLevelTopologyNew) {
            auto trustLine = trustLinePtr->topologyTrustLine();
            auto trustLineFreeAmountShared = trustLine->freeAmount();
            auto trustLineAmountPtr = trustLineFreeAmountShared.get();
            mForbiddenNodeIDs.clear();
            TrustLineAmount flow = calculateOneNodeNew(
                trustLine->targetID(),
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

TrustLineAmount MaxFlowCalculationFullyTransaction::calculateOneNodeNew(
    ContractorID nodeID,
    const TrustLineAmount& currentFlow,
    byte level)
{
    if (nodeID == mCurrentContractorNew) {
        if (currentFlow > TrustLine::kZeroAmount()) {
            mCurrentMaxFlow += currentFlow;
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
                mForbiddenNodeIDs.begin(),
                mForbiddenNodeIDs.end(),
                trustLine->targetID()) != mForbiddenNodeIDs.end()) {
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
        mForbiddenNodeIDs.push_back(nodeID);
        TrustLineAmount calcFlow = calculateOneNodeNew(
            trustLine->targetID(),
            nextFlow,
            level + (byte)1);
        mForbiddenNodeIDs.pop_back();
        if (calcFlow > TrustLine::kZeroAmount()) {
            trustLine->addUsedAmount(calcFlow);
            return calcFlow;
        }
    }
    return 0;
}

TransactionResult::SharedConst MaxFlowCalculationFullyTransaction::resultOk()
{
    stringstream ss;
    ss << mMaxFlows.size();
    for (const auto &nodeUUIDAndMaxFlow : mMaxFlows) {
        ss << kTokensSeparator << nodeUUIDAndMaxFlow.first << kTokensSeparator;
        ss << nodeUUIDAndMaxFlow.second;
    }
    stringstream sss;
    sss << mMaxFlowsNew.size();
    for (const auto &nodeIDAndMaxFlow : mMaxFlowsNew) {
        for (const auto &nodeIDAndAddress : mContractorIDs) {
            if (nodeIDAndAddress.first == nodeIDAndMaxFlow.first) {
                sss << kTokensSeparator << nodeIDAndAddress.second->fullAddress()
                    << kTokensSeparator << nodeIDAndMaxFlow.second;
                break;
            }
        }
    }
    info() << "Final old max flows " << ss.str();
    info() << "Final new max flows " << sss.str();
    auto kMaxFlowAmountsStr = sss.str();
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
    s << "[MaxFlowCalculationFullyTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}