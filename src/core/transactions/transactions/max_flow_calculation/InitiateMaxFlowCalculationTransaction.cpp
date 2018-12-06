#include "InitiateMaxFlowCalculationTransaction.h"

InitiateMaxFlowCalculationTransaction::InitiateMaxFlowCalculationTransaction(
    NodeUUID &nodeUUID,
    InitiateMaxFlowCalculationCommand::Shared command,
    ContractorsManager *contractorsManager,
    TrustLinesManager *trustLinesManager,
    TopologyTrustLinesManager *topologyTrustLineManager,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    Logger &logger) :

    BaseCollectTopologyTransaction(
        BaseTransaction::InitiateMaxFlowCalculationTransactionType,
        nodeUUID,
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
    mShortMaxFlowsCalculated(false)
{}

TransactionResult::SharedConst InitiateMaxFlowCalculationTransaction::sendRequestForCollectingTopology()
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
        mMaxFlowsNew.insert(
            make_pair(
                contractorID,
                TrustLineAmount(0)));
    }
    for (const auto &contractorUUID : mCommand->contractors()) {
        mMaxFlows.insert(
            make_pair(
                contractorUUID,
                TrustLineAmount(0)));
    }
    // Check if Node does not have outgoing FlowAmount;
    if(mTrustLinesManager->firstLevelNeighborsWithOutgoingFlow().empty()){
        return resultFinalOk();
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

    vector<BaseAddress::Shared> nonCachedContractorsNew;
    for (const auto &contractorAddress : mCommand->contractorAddresses()) {
        auto nodeCache = mMaxFlowCacheManager->cacheByAddress(contractorAddress);
        if (nodeCache == nullptr) {
            nonCachedContractorsNew.push_back(contractorAddress);
            allContractorsAlreadyCalculated = false;
        } else {
            allContractorsAlreadyCalculated &= nodeCache->isFlowFinal();
        }
    }

    if (allContractorsAlreadyCalculated) {
        for (const auto &contractorUUID : mCommand->contractors()) {
            auto nodeCache = mMaxFlowCacheManager->cacheByNode(contractorUUID);
            mMaxFlows[contractorUUID] = nodeCache->currentFlow();
        }
        for (const auto &contractorAddress : mCommand->contractorAddresses()) {
            auto nodeCache = mMaxFlowCacheManager->cacheByAddress(contractorAddress);
            for (const auto &contractorIDAndAddress : mContractorIDs) {
                if (contractorIDAndAddress.second == contractorAddress) {
                    mMaxFlowsNew[contractorIDAndAddress.first] = nodeCache->currentFlow();
                    break;
                }
            }
        }
        return resultFinalOk();
    }

    const auto kTransaction = make_shared<CollectTopologyTransaction>(
        mNodeUUID,
        mEquivalent,
        nonCachedContractorsNew,
        mContractorsManager,
        mTrustLinesManager,
        mTopologyTrustLineManager,
        mTopologyCacheManager,
        mMaxFlowCacheManager,
        mLog);
    mTopologyTrustLineManager->setPreventDeleting(true);
    launchSubsidiaryTransaction(kTransaction);
    bool isDirectPathOccurred = false;
    for (const auto &contractorUUID : mCommand->contractors()) {
        if (mTrustLinesManager->trustLineIsPresent(contractorUUID)) {
            const auto kOutgoingFlow = mTrustLinesManager->outgoingTrustAmountConsideringReservations(contractorUUID);
            mMaxFlows[contractorUUID] = *kOutgoingFlow;
            isDirectPathOccurred = true;
        }
    }
    for (const auto &contractorIDAndAddress : mContractorIDs) {
        auto neighborID = mContractorsManager->contractorIDByAddress(contractorIDAndAddress.second);
        if (neighborID != ContractorsManager::kNotFoundContractorID) {
            const auto kOutgoingFlow = mTrustLinesManager->outgoingTrustAmountConsideringReservations(neighborID);
            mMaxFlowsNew[contractorIDAndAddress.first] = *kOutgoingFlow;
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
            for (const auto &contractorUUID : mCommand->contractors()) {
                auto outgoingFlowFromGatewayToContractor = mTopologyTrustLineManager->flowAmount(
                    gateway,
                    contractorUUID);
                if (outgoingFlowFromGatewayToContractor != TrustLine::kZeroAmount()) {
                    mMaxFlows[contractorUUID] += min(*outgoingFlowToGateway, outgoingFlowFromGatewayToContractor);
                    gatewayPathOccurred = true;
                }
            }
        }

        for (const auto &gateway : mGatewaysNew) {
            if (!mTrustLinesManager->trustLineIsPresent(gateway)) {
                continue;
            }
            const auto outgoingFlowToGateway = mTrustLinesManager->outgoingTrustAmountConsideringReservations(gateway);
            if (*outgoingFlowToGateway == TrustLine::kZeroAmount()) {
                continue;
            }
            for (const auto &contractorIDAndAddress : mContractorIDs) {
                auto outgoingFlowFromGatewayToContractor = mTopologyTrustLineManager->flowAmountNew(
                    gateway,
                    contractorIDAndAddress.first);
                if (outgoingFlowFromGatewayToContractor != TrustLine::kZeroAmount()) {
                    mMaxFlowsNew[contractorIDAndAddress.first] +=
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
        mTopologyTrustLineManager->printTrustLines();
        mTopologyTrustLineManager->printTrustLinesNew();
        mShortMaxFlowsCalculated = true;
        mFirstLevelTopology =
                mTopologyTrustLineManager->trustLinePtrsSet(mNodeUUID);
        mFirstLevelTopologyNew = mTopologyTrustLineManager->trustLinePtrsSetNew(0);
        mMaxPathLength = kShortMaxPathLength;
        auto startTime = utc_now();
        for (const auto &contractorUUID : mCommand->contractors()) {
            auto nodeCache = mMaxFlowCacheManager->cacheByNode(contractorUUID);
            if (nodeCache != nullptr) {
                mMaxFlows[contractorUUID] =
                    nodeCache->currentFlow();
            } else {
                mMaxFlows[contractorUUID] =
                    calculateMaxFlow(
                        contractorUUID);
            }
        }
        for (const auto &contractorIDAndAddress : mContractorIDs) {
            auto nodeCache = mMaxFlowCacheManager->cacheByAddress(contractorIDAndAddress.second);
            if (nodeCache != nullptr) {
                mMaxFlowsNew[contractorIDAndAddress.first] =
                    nodeCache->currentFlow();
            } else {
                mMaxFlowsNew[contractorIDAndAddress.first] =
                    calculateMaxFlowNew(
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

    mFinalTopologyCollected = contextSize == 0;
    mFirstLevelTopology =
        mTopologyTrustLineManager->trustLinePtrsSet(mNodeUUID);
    mFirstLevelTopologyNew = mTopologyTrustLineManager->trustLinePtrsSetNew(0);
    mMaxPathLength = kLongMaxPathLength;
    mCurrentGlobalContractorIdx = 0;
    mStep = CustomLogic;
    return applyCustomLogic();
}

TransactionResult::SharedConst InitiateMaxFlowCalculationTransaction::applyCustomLogic()
{
    if (mCurrentGlobalContractorIdx >= mCommand->contractors().size()) {
        error() << "Illegal current contractor idx: " << mCurrentGlobalContractorIdx;
        mTopologyTrustLineManager->setPreventDeleting(false);
        return resultFinalOk();
    }
    const auto contractorUUID = mCommand->contractors().at(mCurrentGlobalContractorIdx);
    const auto contractorIDAndAddress = mContractorIDs.at(mCurrentGlobalContractorIdx);
    auto nodeCache = mMaxFlowCacheManager->cacheByNode(contractorUUID);
    // todo : implement separated logic when !nodeCache->isFlowFinal()
    if (nodeCache != nullptr && nodeCache->isFlowFinal()) {
        mMaxFlows[contractorUUID] = nodeCache->currentFlow();
    } else {
        mMaxFlows[contractorUUID] = calculateMaxFlow(
            contractorUUID);
        if (nodeCache != nullptr) {
            mMaxFlowCacheManager->updateCache(
                contractorUUID,
                mMaxFlows[contractorUUID],
                true);
        } else {
            mMaxFlowCacheManager->addCache(
                contractorUUID,
                make_shared<MaxFlowCache>(
                    mMaxFlows[contractorUUID],
                    true));
        }
    }

    auto nodeCacheNew = mMaxFlowCacheManager->cacheByAddress(contractorIDAndAddress.second);
    // todo : implement separated logic when !nodeCache->isFlowFinal()
    if (nodeCacheNew != nullptr && nodeCacheNew->isFlowFinal()) {
        mMaxFlowsNew[contractorIDAndAddress.first] = nodeCacheNew->currentFlow();
    } else {
        mMaxFlowsNew[contractorIDAndAddress.first] = calculateMaxFlowNew(
            contractorIDAndAddress.first);
        if (nodeCache != nullptr) {
            mMaxFlowCacheManager->updateCacheNew(
                contractorIDAndAddress.second,
                mMaxFlowsNew[contractorIDAndAddress.first],
                true);
        } else {
            mMaxFlowCacheManager->addCacheNew(
                contractorIDAndAddress.second,
                make_shared<MaxFlowCache>(
                    mMaxFlowsNew[contractorIDAndAddress.first],
                    true));
        }
    }

    mCurrentGlobalContractorIdx++;
    if (mCurrentGlobalContractorIdx == mCommand->contractors().size()) {
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
    for (mCurrentPathLength = 1; mCurrentPathLength <= mMaxPathLength; mCurrentPathLength++) {
        calculateMaxFlowOnOneLevel();
    }

    mTopologyTrustLineManager->resetAllUsedAmounts();
    info() << contractorUUID << " max flow calculating time: " << utc_now() - startTime;
    return mCurrentMaxFlow;
}

TrustLineAmount InitiateMaxFlowCalculationTransaction::calculateMaxFlowNew(
    ContractorID contractorID)
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "start found flow to: " << contractorID;
    info() << "gateways:";
    for (auto const &gateway : mTopologyTrustLineManager->gatewaysNew()) {
        info() << "\t" << gateway;
    }
#endif
    DateTime startTime = utc_now();

    mTopologyTrustLineManager->makeFullyUsedTLsFromGatewaysToAllNodesExceptOneNew(
        contractorID);
    mCurrentContractorNew = contractorID;
    if (mFirstLevelTopology.empty()) {
        mTopologyTrustLineManager->resetAllUsedAmounts();
        return TrustLine::kZeroAmount();
    }

    mCurrentMaxFlow = TrustLine::kZeroAmount();
    for (mCurrentPathLength = 1; mCurrentPathLength <= mMaxPathLength; mCurrentPathLength++) {
        calculateMaxFlowOnOneLevelNew();
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

void InitiateMaxFlowCalculationTransaction::calculateMaxFlowOnOneLevelNew()
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

TrustLineAmount InitiateMaxFlowCalculationTransaction::calculateOneNodeNew(
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

TransactionResult::SharedConst InitiateMaxFlowCalculationTransaction::resultFinalOk()
{
    stringstream ss;
    ss << kFinalStep << kTokensSeparator << mMaxFlows.size();
    for (const auto &nodeUUIDAndMaxFlow : mMaxFlows) {
        ss << kTokensSeparator << nodeUUIDAndMaxFlow.first << kTokensSeparator;
        ss << nodeUUIDAndMaxFlow.second;
    }
    stringstream sss;
    sss << kFinalStep << kTokensSeparator << mMaxFlowsNew.size();
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

TransactionResult::SharedConst InitiateMaxFlowCalculationTransaction::resultIntermediateOk()
{
    uint16_t stepMaxFlows = mResultStep;
    mResultStep++;
    stringstream ss;
    ss << stepMaxFlows << kTokensSeparator << mMaxFlows.size();
    for (const auto &nodeUUIDAndMaxFlow : mMaxFlows) {
        ss << kTokensSeparator << nodeUUIDAndMaxFlow.first << kTokensSeparator;
        ss << nodeUUIDAndMaxFlow.second;
    }
    stringstream sss;
    sss << stepMaxFlows << kTokensSeparator << mMaxFlowsNew.size();
    for (const auto &nodeIDAndMaxFlow : mMaxFlowsNew) {
        for (const auto &nodeIDAndAddress : mContractorIDs) {
            if (nodeIDAndAddress.first == nodeIDAndMaxFlow.first) {
                sss << kTokensSeparator << nodeIDAndAddress.second->fullAddress()
                    << kTokensSeparator << nodeIDAndMaxFlow.second;
                break;
            }
        }
    }
    info() << "Intermediate old max flows " << ss.str();
    info() << "Intermediate new max flows " << sss.str();
    auto kMaxFlowAmountsStr = sss.str();
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