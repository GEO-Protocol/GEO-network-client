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
    info() << "run\t" << "targets count: " << mCommand->contractorAddresses().size();
    info() << "SendRequestForCollectingTopology";
#endif
    info() << "Contractors addresses:";
    for (const auto &contractor : mCommand->contractorAddresses()) {
        info() << contractor->fullAddress();
    }

    auto ownAddress = mContractorsManager->ownAddresses().at(0);
    for (const auto &contractorAddress : mCommand->contractorAddresses()) {
        if (contractorAddress == ownAddress) {
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
        mMaxFlows.reserve(mCommand->contractorAddresses().size());
        for (const auto &contractorIDAndAddress : mContractorIDs) {
            mMaxFlows.emplace_back(
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
    mMaxFlows.reserve(mCommand->contractorAddresses().size());
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    mTopologyTrustLineManager->printTrustLines();
#endif
    mFirstLevelTopology = mTopologyTrustLineManager->trustLinePtrsSet(
        TopologyTrustLinesManager::kCurrentNodeID);
    mCurrentGlobalContractorIdx = 0;
    mStep = CustomLogic;
    return applyCustomLogic();
}

TransactionResult::SharedConst MaxFlowCalculationFullyTransaction::applyCustomLogic()
{
    if (mCurrentGlobalContractorIdx >= mCommand->contractorAddresses().size()) {
        error() << "Illegal current contractor idx: " << mCurrentGlobalContractorIdx;
        mTopologyTrustLineManager->setPreventDeleting(false);
        return resultOk();
    }
    const auto contractorIDAndAddress = mContractorIDs.at(mCurrentGlobalContractorIdx);
    auto maxFlow = calculateMaxFlow(
        contractorIDAndAddress.first);
    mMaxFlows.emplace_back(
        contractorIDAndAddress.first,
        maxFlow);
    auto nodeCacheNew = mMaxFlowCacheManager->cacheByAddress(contractorIDAndAddress.second);
    if (nodeCacheNew != nullptr) {
        mMaxFlowCacheManager->updateCache(
            contractorIDAndAddress.second,
            maxFlow,
            true);
    } else {
        mMaxFlowCacheManager->addCache(
            contractorIDAndAddress.second,
            make_shared<MaxFlowCache>(
                maxFlow,
                true));
    }
    if (mMaxFlows.size() >= mCommand->contractorAddresses().size()) {
        mTopologyTrustLineManager->setPreventDeleting(false);
        return resultOk();
    }
    mCurrentGlobalContractorIdx++;
    return resultAwakeAsFastAsPossible();
}

// this method used the same logic as PathsManager::reBuildPaths
// and PathsManager::buildPaths
TrustLineAmount MaxFlowCalculationFullyTransaction::calculateMaxFlow(
    ContractorID contractorID)
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "calculateMaxFlowNew\tstart found flow to: " << contractorID;
#endif
    DateTime startTime = utc_now();

    mTopologyTrustLineManager->makeFullyUsedTLsFromGatewaysToAllNodesExceptOne(
        contractorID);

    mCurrentContractor = contractorID;
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
            mForbiddenNodeIDs.clear();
            TrustLineAmount flow = calculateOneNode(
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
    ContractorID nodeID,
    const TrustLineAmount &currentFlow,
    byte level)
{
    if (nodeID == mCurrentContractor) {
        if (currentFlow > TrustLine::kZeroAmount()) {
            mCurrentMaxFlow += currentFlow;
        }
        return currentFlow;
    }
    if (level == mCurrentPathLength) {
        return 0;
    }

    auto trustLinePtrsSet = mTopologyTrustLineManager->trustLinePtrsSet(nodeID);
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
        TrustLineAmount calcFlow = calculateOneNode(
            trustLine->targetID(),
            nextFlow,
            level + (byte) 1);
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
    for (const auto &nodeIDAndMaxFlow : mMaxFlows) {
        for (const auto &nodeIDAndAddress : mContractorIDs) {
            if (nodeIDAndAddress.first == nodeIDAndMaxFlow.first) {
                ss << kTokensSeparator << nodeIDAndAddress.second->fullAddress()
                    << kTokensSeparator << nodeIDAndMaxFlow.second;
                break;
            }
        }
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
    s << "[MaxFlowCalculationFullyTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}