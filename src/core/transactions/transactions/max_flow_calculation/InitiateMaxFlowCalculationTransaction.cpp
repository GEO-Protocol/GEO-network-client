#include "InitiateMaxFlowCalculationTransaction.h"

InitiateMaxFlowCalculationTransaction::InitiateMaxFlowCalculationTransaction(
    NodeUUID &nodeUUID,
    InitiateMaxFlowCalculationCommand::Shared command,
    TrustLinesManager *manager,
    MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::InitiateMaxFlowCalculationTransactionType,
        nodeUUID,
        logger),
    mCommand(command),
    mTrustLinesManager(manager),
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager),
    mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager)
{}

InitiateMaxFlowCalculationCommand::Shared InitiateMaxFlowCalculationTransaction::command() const
{
    return mCommand;
}

TransactionResult::SharedConst InitiateMaxFlowCalculationTransaction::run()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "run\t" << "initiator: " << mNodeUUID;
    info() << "run\t" << "targets count: " << mCommand->contractors().size();
#endif
    switch (mStep) {
        case Stages::SendRequestForCollectingTopology: {
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
            info() << "SendRequestForCollectingTopology";
#endif
            // Check if there is mNodeUUID in command parameters
            for (const auto &contractorUUID : mCommand->contractors()) {
                if (contractorUUID == currentNodeUUID()) {
                    error() << "Attempt to initialise operation against itself was prevented. Canceled.";
                    return resultProtocolError();
                }
            }
            // Check if Node does not have outgoing FlowAmount;
            if(mTrustLinesManager->firstLevelNeighborsWithOutgoingFlow().size() == 0){
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
            if (!mMaxFlowCalculationCacheManager->isInitiatorCached()) {
                for (auto const &nodeUUIDAndOutgoingFlow : mTrustLinesManager->outgoingFlows()) {
                    auto trustLineAmountShared = nodeUUIDAndOutgoingFlow.second;
                    mMaxFlowCalculationTrustLineManager->addTrustLine(
                        make_shared<MaxFlowCalculationTrustLine>(
                            mNodeUUID,
                            nodeUUIDAndOutgoingFlow.first,
                            trustLineAmountShared));
                }
                sendMessagesOnFirstLevel();
                mMaxFlowCalculationCacheManager->setInitiatorCache();
            }
            sendMessagesToContractors();
            mMaxFlowCalculationTrustLineManager->setPreventDeleting(true);
            mStep = Stages::CalculateMaxTransactionFlow;
            return resultAwaikAfterMilliseconds(
                kWaitMilisecondsForCalculatingMaxFlow);
        }
        case Stages::CalculateMaxTransactionFlow: {
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
            info() << "CalculateMaxTransactionFlow";
#endif
            vector<pair<NodeUUID, TrustLineAmount>> maxFlows;
            maxFlows.reserve(mCommand->contractors().size());
            for (const auto &contractorUUID : mCommand->contractors()) {
                maxFlows.push_back(
                    make_pair(
                        contractorUUID,
                        calculateMaxFlow(
                            contractorUUID)));
            }
            mMaxFlowCalculationTrustLineManager->setPreventDeleting(false);
            mStep = Stages::SendRequestForCollectingTopology;
            return resultOk(maxFlows);
        }
        default:
            throw ValueError("InitiateMaxFlowCalculationTransaction::run: "
                                 "wrong value of mStep");
    }
}

void InitiateMaxFlowCalculationTransaction::sendMessagesToContractors()
{
    for (const auto &contractorUUID : mCommand->contractors())
    sendMessage<InitiateMaxFlowCalculationMessage>(
        contractorUUID,
        currentNodeUUID());
}

void InitiateMaxFlowCalculationTransaction::sendMessagesOnFirstLevel()
{
    vector<NodeUUID> outgoingFlowUuids = mTrustLinesManager->firstLevelNeighborsWithOutgoingFlow();
    for (auto const &nodeUUIDOutgoingFlow : outgoingFlowUuids) {
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
        info() << "sendFirst\t" << nodeUUIDOutgoingFlow;
#endif
        sendMessage<MaxFlowCalculationSourceFstLevelMessage>(
            nodeUUIDOutgoingFlow,
            mNodeUUID);
    }
}

// this method used the same logic as PathsManager::reBuildPaths
// and PathsManager::buildPaths
TrustLineAmount InitiateMaxFlowCalculationTransaction::calculateMaxFlow(
    const NodeUUID &contractorUUID)
{
    auto startTime = utc_now();
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "calculateMaxFlow\tstart found flow to: " << contractorUUID;
#endif

    mCurrentContractor = contractorUUID;
    auto trustLinePtrsSet =
            mMaxFlowCalculationTrustLineManager->trustLinePtrsSet(mNodeUUID);
    if (trustLinePtrsSet.size() == 0) {
        mMaxFlowCalculationTrustLineManager->resetAllUsedAmounts();
        return TrustLine::kZeroAmount();
    }

    mCurrentMaxFlow = TrustLine::kZeroAmount();
    for (mCurrentPathLength = 1; mCurrentPathLength <= kMaxPathLength; mCurrentPathLength++) {
        calculateMaxFlowOnOneLevel();
    }

    mMaxFlowCalculationTrustLineManager->resetAllUsedAmounts();
    debug() << "max flow calculation time is " << utc_now() - startTime;
    return mCurrentMaxFlow;
}

// this method used the same logic as PathsManager::reBuildPathsOnOneLevel
// and PathsManager::buildPathsOnOneLevel
void InitiateMaxFlowCalculationTransaction::calculateMaxFlowOnOneLevel()
{
    auto trustLinePtrsSet =
            mMaxFlowCalculationTrustLineManager->trustLinePtrsSet(mNodeUUID);
    while(true) {
        TrustLineAmount currentFlow = 0;
        for (auto &trustLinePtr : trustLinePtrsSet) {
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
                break;
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
            info() << "add flow " << currentFlow;
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

TransactionResult::SharedConst InitiateMaxFlowCalculationTransaction::resultOk(
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

TransactionResult::SharedConst InitiateMaxFlowCalculationTransaction::resultProtocolError()
{
    return transactionResultFromCommand(
        mCommand->responseProtocolError());
}

const string InitiateMaxFlowCalculationTransaction::logHeader() const
{
    stringstream s;
    s << "[InitiateMaxFlowCalculationTA: " << currentTransactionUUID() << "]";
    return s.str();
}