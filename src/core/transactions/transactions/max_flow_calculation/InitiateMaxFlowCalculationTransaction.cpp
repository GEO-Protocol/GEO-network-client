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
        mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager) {}

InitiateMaxFlowCalculationCommand::Shared InitiateMaxFlowCalculationTransaction::command() const {

    return mCommand;
}

TransactionResult::SharedConst InitiateMaxFlowCalculationTransaction::run() {

    info() << "run\t" << "initiator: " << mNodeUUID.stringUUID();
    info() << "run\t" << "target: " << mCommand->contractorUUID().stringUUID();
    info() << "run\t" << "trustLineMap size: " << mMaxFlowCalculationTrustLineManager->msTrustLines.size();

    switch (mStep) {
        case Stages::SendRequestForCollectingTopology:
            info() << "start";
            if (!mMaxFlowCalculationCacheManager->isInitiatorCached()) {
                for (auto const &nodeUUIDAndTrustLine : mTrustLinesManager->outgoingFlows()) {
                    mMaxFlowCalculationTrustLineManager->addTrustLine(
                        make_shared<MaxFlowCalculationTrustLine>(
                            mNodeUUID,
                            nodeUUIDAndTrustLine.first,
                            nodeUUIDAndTrustLine.second));
                }
                sendMessagesOnFirstLevel();
                mMaxFlowCalculationCacheManager->setInitiatorCache();

                info() << "run\t" << "step " << mStep;
            }
            sendMessageToRemoteNode();
            mStep = Stages::CalculateMaxTransactionFlow;
            return make_shared<TransactionResult>(
                TransactionState::awakeAfterMilliseconds(kWaitMilisecondsForCalculatingMaxFlow));;
        case Stages::CalculateMaxTransactionFlow:
            TrustLineAmount maxFlow = calculateMaxFlow(mCommand->contractorUUID());
            info() << "run\t" << "max flow: " << maxFlow;
            mStep = Stages::SendRequestForCollectingTopology;
            return resultOk(maxFlow);
    }

}

void InitiateMaxFlowCalculationTransaction::sendMessageToRemoteNode() {

    sendMessage<InitiateMaxFlowCalculationMessage>(
        mCommand->contractorUUID(),
        mNodeUUID);
}

void InitiateMaxFlowCalculationTransaction::sendMessagesOnFirstLevel() {

    vector<NodeUUID> outgoingFlowUuids = mTrustLinesManager->firstLevelNeighborsWithOutgoingFlow();
    for (auto const &nodeUUIDOutgoingFlow : outgoingFlowUuids) {

        info() << "sendFirst\t" << nodeUUIDOutgoingFlow.stringUUID();

        sendMessage<MaxFlowCalculationSourceFstLevelMessage>(
            nodeUUIDOutgoingFlow,
            mNodeUUID);
    }

}

TrustLineAmount InitiateMaxFlowCalculationTransaction::calculateMaxFlow(const NodeUUID& nodeUUID) {
    TrustLineAmount result = 0;
    info() << "calculateMaxFlow\tstart found flow to: " << nodeUUID.stringUUID();
    while(true) {
        vector<MaxFlowCalculationTrustLine::Shared> sortedTrustLines =
                mMaxFlowCalculationTrustLineManager->sortedTrustLines(mNodeUUID);

        if (sortedTrustLines.size() == 0) {
            return result;
        }
        MaxFlowCalculationTrustLine::Shared &trustLineMax = *sortedTrustLines.begin();
        auto trustLineFreeAmountPtr = trustLineMax.get()->freeAmount();
        if (*trustLineFreeAmountPtr == TrustLine::kZeroAmount()) {
            return result;
        }
        info() << "calculateMaxFlow\t" << "1st max flow: " << ((uint32_t)*trustLineFreeAmountPtr);

        TrustLineAmount currentFlow = 0;
        for (auto &trustLine : sortedTrustLines) {
            auto trustLineFreeAmountShared = trustLine.get()->freeAmount();
            auto trustLineAmountPtr = trustLineFreeAmountShared.get();
            set<NodeUUID> forbiddenUUIDs;
            TrustLineAmount flow = calculateOneNode(
                trustLine.get()->targetUUID(),
                *trustLineAmountPtr,
                1,
                nodeUUID,
                mNodeUUID,
                forbiddenUUIDs);
            if (flow > TrustLine::kZeroAmount()) {
                info() << "calculateMaxFlow\t" << "used flow: " << trustLine.get()->sourceUUID().stringUUID() << "->"
                              << trustLine.get()->targetUUID().stringUUID() << " " << flow;
                currentFlow += flow;
                trustLine->addUsedAmount(flow);
                auto trustLineFreeAmountTmp = trustLine.get()->freeAmount();
                auto trustLineAmountTmp = trustLineFreeAmountTmp.get();
                info() << "calculateMaxFlow\t" << "new flow: " << (uint32_t)*trustLineAmountTmp;
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
    byte level,
    const NodeUUID& targetUUID,
    const NodeUUID& sourceUUID,
    set<NodeUUID> forbiddenNodeUUIDs) {

    info() << "calculateMaxFlow\t" << "go in: " << nodeUUID.stringUUID() << "->"
                  << currentFlow << "->" << to_string(level);
    info() << "calculateMaxFlow\t" << "forbidden nodes: " << forbiddenNodeUUIDs.size();
    if (nodeUUID == targetUUID) {
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
        if (trustLine.get()->targetUUID() == sourceUUID) {
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
            level + 1,
            targetUUID,
            sourceUUID,
            forbiddenNodeUUIDs);
        forbiddenNodeUUIDs.erase(nodeUUID);
        if (calcFlow > TrustLine::kZeroAmount()) {
            info() << "calculateMaxFlow\t" << "used flow: " << trustLine.get()->sourceUUID().stringUUID() << "->"
                          << trustLine.get()->targetUUID().stringUUID() << " " << calcFlow;
            trustLine->addUsedAmount(calcFlow);
            auto trustLineFreeAmountTmp = trustLine.get()->freeAmount();
            auto trustLineAmountTmp = trustLineFreeAmountTmp.get();
            info() << "calculateMaxFlow\t" << "new flow: " << (uint32_t)*trustLineAmountTmp;
            return calcFlow;
        }
    }
    return 0;
}

TransactionResult::SharedConst InitiateMaxFlowCalculationTransaction::resultOk(TrustLineAmount &maxFlowAmount) {

    string maxFlowAmountStr = to_string((uint32_t)maxFlowAmount);
    return transactionResultFromCommand(mCommand->resultOk(maxFlowAmountStr));
}

const string InitiateMaxFlowCalculationTransaction::logHeader() const
{
    stringstream s;
    s << "[InitiateMaxFlowCalculationTA]";

    return s.str();
}