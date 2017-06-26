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

TrustLineAmount InitiateMaxFlowCalculationTransaction::calculateMaxFlow(
    const NodeUUID &contractorUUID)
{
    TrustLineAmount result = 0;
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "calculateMaxFlow\tstart found flow to: " << contractorUUID;
#endif
    auto trustLinePtrsSet =
        mMaxFlowCalculationTrustLineManager->trustLinePtrsSet(mNodeUUID);
    if (trustLinePtrsSet.size() == 0) {
        mMaxFlowCalculationTrustLineManager->resetAllUsedAmounts();
        return result;
    }

    while(true) {
        TrustLineAmount currentFlow = 0;
        for (auto &trustLinePtr : trustLinePtrsSet) {
            auto trustLine = trustLinePtr->maxFlowCalculationtrustLine();
            auto trustLineFreeAmountShared = trustLine.get()->freeAmount();
            auto trustLineAmountPtr = trustLineFreeAmountShared.get();
            forbiddenNodeUUIDs.clear();
            TrustLineAmount flow = calculateOneNode(
                trustLine.get()->targetUUID(),
                contractorUUID,
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
    mMaxFlowCalculationTrustLineManager->resetAllUsedAmounts();
    return result;
}

TrustLineAmount InitiateMaxFlowCalculationTransaction::calculateOneNode(
    const NodeUUID& nodeUUID,
    const NodeUUID& contractorUUID,
    const TrustLineAmount& currentFlow,
    byte level)
{
    if (nodeUUID == contractorUUID) {
        return currentFlow;
    }
    if (level == kMaxFlowLength) {
        return 0;
    }

    auto trustLinePtrsSet =
            mMaxFlowCalculationTrustLineManager->trustLinePtrsSet(nodeUUID);
    if (trustLinePtrsSet.size() == 0) {
        return 0;
    }
    for (auto &trustLinePtr : trustLinePtrsSet) {
        auto trustLine = trustLinePtr->maxFlowCalculationtrustLine();
        if (trustLine.get()->targetUUID() == mNodeUUID) {
            continue;
        }
        if (forbiddenNodeUUIDs.find(trustLine.get()->targetUUID()) != forbiddenNodeUUIDs.end()) {
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
        forbiddenNodeUUIDs.insert(nodeUUID);
        TrustLineAmount calcFlow = calculateOneNode(
            trustLine.get()->targetUUID(),
            contractorUUID,
            nextFlow,
            level + (byte)1);
        forbiddenNodeUUIDs.erase(nodeUUID);
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