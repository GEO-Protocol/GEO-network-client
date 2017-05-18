#include "InitiateMaxFlowCalculationTransaction.h"

InitiateMaxFlowCalculationTransaction::InitiateMaxFlowCalculationTransaction(
    NodeUUID &nodeUUID,
    InitiateMaxFlowCalculationCommand::Shared command,
    TrustLinesManager *manager,
    MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    Logger *logger) :

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
    info() << "run\t" << "target: " << mCommand->contractorUUID();
#endif
    switch (mStep) {
        case Stages::SendRequestForCollectingTopology: {
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
            info() << "start";
#endif
            if (mCommand->contractorUUID() == currentNodeUUID()) {
                error() << "Attempt to initialise operation against itself was prevented. Canceled.";
                return resultProtocolError();
            }
            if (!mMaxFlowCalculationCacheManager->isInitiatorCached()) {
                for (auto const &nodeUUIDAndTrustLine : mTrustLinesManager->outgoingFlows()) {
                    auto trustLineAmountShared = nodeUUIDAndTrustLine.second;
                    mMaxFlowCalculationTrustLineManager->addTrustLine(
                        make_shared<MaxFlowCalculationTrustLine>(
                            mNodeUUID,
                            nodeUUIDAndTrustLine.first,
                            trustLineAmountShared));
                }
                sendMessagesOnFirstLevel();
                mMaxFlowCalculationCacheManager->setInitiatorCache();
            }
            sendMessageToRemoteNode();
            mStep = Stages::CalculateMaxTransactionFlow;
            return make_shared<TransactionResult>(
                TransactionState::awakeAfterMilliseconds(
                    kWaitMilisecondsForCalculatingMaxFlow));
        }
        case Stages::CalculateMaxTransactionFlow: {
            TrustLineAmount maxFlow = calculateMaxFlow();
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
            info() << "run\t" << "max flow: " << maxFlow;
#endif
            mStep = Stages::SendRequestForCollectingTopology;
            return resultOk(maxFlow);
        }
        default:
            throw ValueError("InitiateMaxFlowCalculationTransaction::run: "
                                 "wrong value of mStep");
    }
}

void InitiateMaxFlowCalculationTransaction::sendMessageToRemoteNode()
{
    sendMessage<InitiateMaxFlowCalculationMessage>(
        mCommand->contractorUUID(),
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

TrustLineAmount InitiateMaxFlowCalculationTransaction::calculateMaxFlow()
{
    TrustLineAmount result = 0;
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "calculateMaxFlow\tstart found flow to: " << mCommand->contractorUUID();
    mMaxFlowCalculationTrustLineManager->printTrustLines();
#endif
    while(true) {
        auto sortedTrustLines =
                mMaxFlowCalculationTrustLineManager->sortedTrustLines(mNodeUUID);

#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
        info() << "sorted trustLines: " << sortedTrustLines.size();
        for (auto const trLine : sortedTrustLines) {
            info() << trLine->sourceUUID() << " " << trLine->targetUUID() << " "
                   << *trLine->amount().get() << " " << *trLine->freeAmount();
        }
#endif

        if (sortedTrustLines.size() == 0) {
            mMaxFlowCalculationTrustLineManager->resetAllUsedAmounts();
            return result;
        }
        MaxFlowCalculationTrustLine::Shared &trustLineMax = *sortedTrustLines.begin();
        auto trustLineFreeAmountPtr = trustLineMax.get()->freeAmount();
        if (*trustLineFreeAmountPtr == TrustLine::kZeroAmount()) {
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
            info() << "first trustline free amount is equal zero";
#endif
            mMaxFlowCalculationTrustLineManager->resetAllUsedAmounts();
            return result;
        }
        TrustLineAmount currentFlow = 0;
        for (auto &trustLine : sortedTrustLines) {
            auto trustLineFreeAmountShared = trustLine.get()->freeAmount();
            auto trustLineAmountPtr = trustLineFreeAmountShared.get();
            forbiddenNodeUUIDs.clear();
            TrustLineAmount flow = calculateOneNode(
                trustLine.get()->targetUUID(),
                *trustLineAmountPtr,
                1);
            if (flow > TrustLine::kZeroAmount()) {
                currentFlow += flow;
                trustLine->addUsedAmount(flow);
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
                info() << "calculateMaxFlow\t" << "used flow: " << trustLine.get()->sourceUUID() << "->"
                              << trustLine.get()->targetUUID() << " " << flow;
                auto trustLineFreeAmountTmp = trustLine.get()->freeAmount();
                auto trustLineAmountTmp = trustLineFreeAmountTmp.get();
                info() << "calculateMaxFlow\t" << "new flow: " << *trustLineAmountTmp;
#endif
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
    const TrustLineAmount& currentFlow,
    byte level)
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "calculateMaxFlow\t" << "go in: " << nodeUUID << "->"
                  << currentFlow << "->" << to_string(level);
    info() << "calculateMaxFlow\t" << "forbidden nodes: " << forbiddenNodeUUIDs.size();
#endif
    if (nodeUUID == mCommand->contractorUUID()) {
        return currentFlow;
    }
    if (level == kMaxFlowLength) {
        return 0;
    }
    vector<MaxFlowCalculationTrustLine::Shared> sortedTrustLines =
            mMaxFlowCalculationTrustLineManager->sortedTrustLines(nodeUUID);
    if (sortedTrustLines.size() == 0) {
        return 0;
    }
    for (auto &trustLine : sortedTrustLines) {
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
            nextFlow,
            level + (byte)1);
        forbiddenNodeUUIDs.erase(nodeUUID);
        if (calcFlow > TrustLine::kZeroAmount()) {
            trustLine->addUsedAmount(calcFlow);
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
            info() << "calculateMaxFlow\t" << "used flow: " << trustLine.get()->sourceUUID() << "->"
                          << trustLine.get()->targetUUID() << " " << calcFlow;
            auto trustLineFreeAmountTmp = trustLine.get()->freeAmount();
            auto trustLineAmountTmp = trustLineFreeAmountTmp.get();
            info() << "calculateMaxFlow\t" << "new flow: " << *trustLineAmountTmp;
#endif
            return calcFlow;
        }
    }
    return 0;
}

TransactionResult::SharedConst InitiateMaxFlowCalculationTransaction::resultOk(
    TrustLineAmount &maxFlowAmount)
{
    stringstream ss;
    ss << maxFlowAmount;
    auto kMaxFlowAmountStr = ss.str();
    return transactionResultFromCommand(mCommand->responseOk(kMaxFlowAmountStr));
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