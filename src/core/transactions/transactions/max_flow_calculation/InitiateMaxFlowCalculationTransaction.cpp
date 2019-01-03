#include "InitiateMaxFlowCalculationTransaction.h"

InitiateMaxFlowCalculationTransaction::InitiateMaxFlowCalculationTransaction(
    InitiateMaxFlowCalculationCommand::Shared command,
    ContractorsManager *contractorsManager,
    TrustLinesManager *trustLinesManager,
    TopologyTrustLinesManager *topologyTrustLineManager,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    bool iAmGateway,
    Logger &logger) :

    BaseCollectTopologyTransaction(
        BaseTransaction::InitiateMaxFlowCalculationTransactionType,
        command->equivalent(),
        contractorsManager,
        trustLinesManager,
        topologyTrustLineManager,
        topologyCacheManager,
        maxFlowCacheManager,
        logger),
    mCommand(command),
    mResultStep(1),
    mGatewayResponseProcessed(false),
    mShortMaxFlowsCalculated(false),
    mIamGateway(iAmGateway)
{}

TransactionResult::SharedConst InitiateMaxFlowCalculationTransaction::sendRequestForCollectingTopology()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "run\t" << "targets count: " << mCommand->contractorAddresses().size();
    info() << "SendRequestForCollectingTopology";
#endif
    info() << "contractors addresses:";
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
        mMaxFlows.insert(
            make_pair(
                contractorID,
                TrustLineAmount(0)));
    }
    // Check if Node does not have outgoing FlowAmount;
    if(mTrustLinesManager->firstLevelNeighborsWithOutgoingFlow().empty()){
        return resultFinalOk();
    }

    vector<BaseAddress::Shared> nonCachedContractors;
    bool allContractorsAlreadyCalculated = true;
    for (const auto &contractorAddress : mCommand->contractorAddresses()) {
        auto nodeCache = mMaxFlowCacheManager->cacheByAddress(contractorAddress);
        if (nodeCache == nullptr) {
            nonCachedContractors.push_back(contractorAddress);
            allContractorsAlreadyCalculated = false;
        } else {
            allContractorsAlreadyCalculated &= nodeCache->isFlowFinal();
        }
    }

    if (allContractorsAlreadyCalculated) {
        for (const auto &contractorAddress : mCommand->contractorAddresses()) {
            auto nodeCache = mMaxFlowCacheManager->cacheByAddress(contractorAddress);
            for (const auto &contractorIDAndAddress : mContractorIDs) {
                if (contractorIDAndAddress.second == contractorAddress) {
                    mMaxFlows[contractorIDAndAddress.first] = nodeCache->currentFlow();
                    break;
                }
            }
        }
        return resultFinalOk();
    }

    const auto kTransaction = make_shared<CollectTopologyTransaction>(
        mEquivalent,
        nonCachedContractors,
        mContractorsManager,
        mTrustLinesManager,
        mTopologyTrustLineManager,
        mTopologyCacheManager,
        mMaxFlowCacheManager,
        mIamGateway,
        mLog);
    mTopologyTrustLineManager->setPreventDeleting(true);
    launchSubsidiaryTransaction(kTransaction);
    bool isDirectPathOccurred = false;
    for (const auto &contractorIDAndAddress : mContractorIDs) {
        auto neighborID = mContractorsManager->contractorIDByAddress(contractorIDAndAddress.second);
        if (neighborID != ContractorsManager::kNotFoundContractorID) {
            const auto kOutgoingFlow = mTrustLinesManager->outgoingTrustAmountConsideringReservations(neighborID);
            mMaxFlows[contractorIDAndAddress.first] = *kOutgoingFlow;
            isDirectPathOccurred = true;
        }
    }
    if (isDirectPathOccurred) {
        return resultIntermediateOk();
    } else {
        return resultAwakeAfterMilliseconds(
            kWaitMillisecondsForCalculatingMaxFlow);
    }
}

TransactionResult::SharedConst InitiateMaxFlowCalculationTransaction::processCollectingTopology()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "CalculateMaxTransactionFlow";
    info() << "context size: " << mContext.size();
#endif
    auto const contextSize = mContext.size();
    fillTopology();
    if (!mGatewayResponseProcessed) {
        mGatewayResponseProcessed = true;
        bool gatewayPathOccurred = false;

        for (const auto &gateway : mGateways) {
            if (!mTrustLinesManager->trustLineIsPresent(gateway)) {
                continue;
            }
            const auto outgoingFlowToGateway = mTrustLinesManager->outgoingTrustAmountConsideringReservations(gateway);
            if (*outgoingFlowToGateway == TrustLine::kZeroAmount()) {
                continue;
            }
            for (const auto &contractorIDAndAddress : mContractorIDs) {
                auto outgoingFlowFromGatewayToContractor = mTopologyTrustLineManager->flowAmount(
                    gateway,
                    contractorIDAndAddress.first);
                if (outgoingFlowFromGatewayToContractor != TrustLine::kZeroAmount()) {
                    mMaxFlows[contractorIDAndAddress.first] +=
                            min(*outgoingFlowToGateway, outgoingFlowFromGatewayToContractor);
                    gatewayPathOccurred = true;
                }
            }
        }

        if (gatewayPathOccurred) {
            return resultIntermediateOk();
        } else {
            return resultAwakeAfterMilliseconds(
                kWaitMillisecondsForCalculatingMaxFlow);
        }
    }

    if (!mShortMaxFlowsCalculated) {
        mShortMaxFlowsCalculated = true;
        mFirstLevelTopology = mTopologyTrustLineManager->trustLinePtrsSet(
            TopologyTrustLinesManager::kCurrentNodeID);
        mMaxPathLength = kShortMaxPathLength;
        auto startTime = utc_now();
        for (const auto &contractorIDAndAddress : mContractorIDs) {
            auto nodeCache = mMaxFlowCacheManager->cacheByAddress(contractorIDAndAddress.second);
            if (nodeCache != nullptr) {
                mMaxFlows[contractorIDAndAddress.first] =
                    nodeCache->currentFlow();
            } else {
                mMaxFlows[contractorIDAndAddress.first] =
                    calculateMaxFlow(
                        contractorIDAndAddress.first);
            }
        }
        info() << "all contractors calculating time: " << utc_now() - startTime;
        mCountProcessCollectingTopologyRun = 0;
        return resultIntermediateOk();
    }

    mCountProcessCollectingTopologyRun++;
    if (contextSize > 0 && mCountProcessCollectingTopologyRun <= kCountRunningProcessCollectingTopologyStage) {
        return resultAwakeAfterMilliseconds(
            kWaitMillisecondsForCalculatingMaxFlowAgain);
    }

#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    mTopologyTrustLineManager->printTrustLines();
#endif
    mFinalTopologyCollected = contextSize == 0;
    mFirstLevelTopology = mTopologyTrustLineManager->trustLinePtrsSet(
        TopologyTrustLinesManager::kCurrentNodeID);
    mMaxPathLength = kLongMaxPathLength;
    mCurrentGlobalContractorIdx = 0;
    mStep = CustomLogic;
    return applyCustomLogic();
}

TransactionResult::SharedConst InitiateMaxFlowCalculationTransaction::applyCustomLogic()
{
    if (mCurrentGlobalContractorIdx >= mCommand->contractorAddresses().size()) {
        error() << "Illegal current contractor idx: " << mCurrentGlobalContractorIdx;
        mTopologyTrustLineManager->setPreventDeleting(false);
        return resultFinalOk();
    }

    const auto contractorIDAndAddress = mContractorIDs.at(mCurrentGlobalContractorIdx);
    auto nodeCache = mMaxFlowCacheManager->cacheByAddress(contractorIDAndAddress.second);
    // todo : implement separated logic when !nodeCache->isFlowFinal()
    if (nodeCache != nullptr && nodeCache->isFlowFinal()) {
        mMaxFlows[contractorIDAndAddress.first] = nodeCache->currentFlow();
    } else {
        mMaxFlows[contractorIDAndAddress.first] = calculateMaxFlow(
            contractorIDAndAddress.first);
        if (nodeCache != nullptr) {
            mMaxFlowCacheManager->updateCache(
                contractorIDAndAddress.second,
                mMaxFlows[contractorIDAndAddress.first],
                true);
        } else {
            mMaxFlowCacheManager->addCache(
                contractorIDAndAddress.second,
                make_shared<MaxFlowCache>(
                    mMaxFlows[contractorIDAndAddress.first],
                    true));
        }
    }

    mCurrentGlobalContractorIdx++;
    if (mCurrentGlobalContractorIdx == mCommand->contractorAddresses().size()) {
        if (!mFinalTopologyCollected) {
            mCountProcessCollectingTopologyRun = 0;
            mStep = ProcessCollectingTopology;
            return resultIntermediateOk();
        }
        mTopologyTrustLineManager->setPreventDeleting(false);
        return resultFinalOk();
    }
    return resultAwakeAsFastAsPossible();
}

// this method used the same logic as PathsManager::reBuildPaths
// and PathsManager::buildPaths
TrustLineAmount InitiateMaxFlowCalculationTransaction::calculateMaxFlow(
    ContractorID contractorID)
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "start found flow to: " << contractorID;
    info() << "gateways:";
    for (auto const &gateway : mTopologyTrustLineManager->gateways()) {
        info() << "\t" << gateway;
    }
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
    for (mCurrentPathLength = 1; mCurrentPathLength <= mMaxPathLength; mCurrentPathLength++) {
        calculateMaxFlowOnOneLevel();
    }

    mTopologyTrustLineManager->resetAllUsedAmounts();
    info() << contractorID << " max flow calculating time: " << utc_now() - startTime;
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
TrustLineAmount InitiateMaxFlowCalculationTransaction::calculateOneNode(
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

TransactionResult::SharedConst InitiateMaxFlowCalculationTransaction::resultFinalOk()
{
    stringstream ss;
    ss << kFinalStep << kTokensSeparator << mMaxFlows.size();
    for (const auto &nodeIDAndMaxFlow : mMaxFlows) {
        for (const auto &nodeIDAndAddress : mContractorIDs) {
            if (nodeIDAndAddress.first == nodeIDAndMaxFlow.first) {
                ss << kTokensSeparator << nodeIDAndAddress.second->addressForCommandResult()
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

TransactionResult::SharedConst InitiateMaxFlowCalculationTransaction::resultIntermediateOk()
{
    uint16_t stepMaxFlows = mResultStep;
    mResultStep++;
    stringstream ss;
    ss << stepMaxFlows << kTokensSeparator << mMaxFlows.size();
    for (const auto &nodeIDAndMaxFlow : mMaxFlows) {
        for (const auto &nodeIDAndAddress : mContractorIDs) {
            if (nodeIDAndAddress.first == nodeIDAndMaxFlow.first) {
                ss << kTokensSeparator << nodeIDAndAddress.second->addressForCommandResult()
                    << kTokensSeparator << nodeIDAndMaxFlow.second;
                break;
            }
        }
    }
    auto kMaxFlowAmountsStr = ss.str();
    return transactionResultFromCommandAndAwakeAfterMilliseconds(
        mCommand->responseOk(
            kMaxFlowAmountsStr),
        kWaitMillisecondsForCalculatingMaxFlow);
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