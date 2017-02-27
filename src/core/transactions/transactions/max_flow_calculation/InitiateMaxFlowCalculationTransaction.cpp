#include "InitiateMaxFlowCalculationTransaction.h"

InitiateMaxFlowCalculationTransaction::InitiateMaxFlowCalculationTransaction(
        NodeUUID &nodeUUID,
        InitiateMaxFlowCalculationCommand::Shared command,
        TrustLinesManager *manager,
        MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
        Logger *logger) :

        BaseTransaction(
                BaseTransaction::TransactionType::InitiateMaxFlowCalculationTransactionType,
                nodeUUID
        ),
        mCommand(command),
        mTrustLinesManager(manager),
        mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager),
        mLog(logger){}

InitiateMaxFlowCalculationCommand::Shared InitiateMaxFlowCalculationTransaction::command() const {

    return mCommand;
}

TransactionResult::SharedConst InitiateMaxFlowCalculationTransaction::run() {

    mLog->logInfo("InitiateMaxFlowCalculationTransaction->run", "initiator: " + mNodeUUID.stringUUID());
    mLog->logInfo("InitiateMaxFlowCalculationTransaction->run", "target: " + mCommand->contractorUUID().stringUUID());
    /*mLog->logInfo("InitiateMaxFlowCalculationTransaction->run",
                  "OutgoingFlows: " + to_string(mTrustLinesManager->getOutgoingFlows().size()));
    for (auto const &nodeUUIDAndTrustLine : mTrustLinesManager->outgoingFlows()) {
        {
            auto info = mLog->info("InitiateMaxFlowCalculationTransaction->run");
            info << "out flow " << nodeUUIDAndTrustLine.first.stringUUID() << "  " <<  nodeUUIDAndTrustLine.second << "\n";
        }
    }

    mLog->logInfo("InitiateMaxFlowCalculationTransaction->run",
                  "IncomingFlows: " + to_string(mTrustLinesManager->getIncomingFlows().size()));
    for (auto const &nodeUUIDAndTrustLine : mTrustLinesManager->getIncomingFlows()) {
        mLog->logInfo("InitiateMaxFlowCalculationTransaction->run", "in flow: " + nodeUUIDAndTrustLine.first.stringUUID()
                                                                    + "  " + to_string((uint32_t)nodeUUIDAndTrustLine.second));
    }*/

    for (auto const &nodeUUIDAndTrustLine : mTrustLinesManager->getOutgoingFlows()) {
        mMaxFlowCalculationTrustLineManager->addTrustLine(
            make_shared<MaxFlowCalculationTrustLine>(
                mNodeUUID,
                nodeUUIDAndTrustLine.first,
                nodeUUIDAndTrustLine.second)
        );
        //mMaxFlowCalculationTrustLineManager->addFlow(mNodeUUID, nodeUUIDAndTrustLine.first, nodeUUIDAndTrustLine.second);
    }
    mLog->logInfo("InitiateMaxFlowCalculationTransaction::run",
                  "trustLineMap size: " + to_string(mMaxFlowCalculationTrustLineManager->mvTrustLines.size()));
    /*for (const auto &it : mMaxFlowCalculationTrustLineManager->mEntities) {
        mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                      "key: " + ((NodeUUID)it.first).stringUUID());

        for (const auto &it1 : it.second->mIncomingFlows) {
            mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                          "inflow: " + ((NodeUUID) it1.first).stringUUID() + " "
                          + to_string((uint32_t)it1.second));
        }

        for (const auto &it1 : it.second->mOutgoingFlows) {
            mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                          "outflow: " + ((NodeUUID) it1.first).stringUUID() + " "
                          + to_string((uint32_t)it1.second));
        }
    }*/

    if (mStep == 1) {
        sendMessageToRemoteNode();
        sendMessageOnFirstLevel();

        mLog->logInfo("InitiateMaxFlowCalculationTransaction::run", "step " + to_string(mStep));
        TrustLineAmount maxFlow = calculateMaxFlow(mCommand->contractorUUID());
        mLog->logInfo("InitiateMaxFlowCalculationTransaction::run",
                      "max flow: " + to_string((uint32_t)maxFlow));

        increaseStepsCounter();
        return make_shared<TransactionResult>(TransactionState::awakeAfterMilliseconds(3000));
    } else {
        TrustLineAmount maxFlow = calculateMaxFlow(mCommand->contractorUUID());
        mLog->logInfo("InitiateMaxFlowCalculationTransaction::run",
                      "max flow: " + to_string((uint32_t)maxFlow));
        return make_shared<TransactionResult>(TransactionState::exit());
    }

}

void InitiateMaxFlowCalculationTransaction::sendMessageToRemoteNode() {

    Message *message = new InitiateMaxFlowCalculationMessage(
        mNodeUUID,
        mNodeUUID);

    addMessage(
        Message::Shared(message),
        mCommand->contractorUUID());
}

void InitiateMaxFlowCalculationTransaction::sendMessageOnFirstLevel() {

    vector<NodeUUID> outgoingFlowUuids = mTrustLinesManager->getFirstLevelNeighborsWithOutgoingFlow();
    for (auto const &it : outgoingFlowUuids) {
        Message *message = new SendMaxFlowCalculationSourceFstLevelMessage(
            mNodeUUID,
            mNodeUUID);

        mLog->logInfo("InitiateMaxFlowCalculationTransaction->sendFirst", ((NodeUUID)it).stringUUID());
        addMessage(
            Message::Shared(message),
            it);
    }

}

TrustLineAmount InitiateMaxFlowCalculationTransaction::calculateMaxFlow(const NodeUUID& nodeUUID) {
    TrustLineAmount result = 0;
    mLog->logInfo("InitiateMaxFlowCalculationTransaction->calculateMaxFlow",
                  "start found flow to: " + nodeUUID.stringUUID());
    while(true) {
        vector<MaxFlowCalculationTrustLine::Shared> sortedTrustLines =
            mMaxFlowCalculationTrustLineManager->getSortedTrustLines(mNodeUUID);
            //mMaxFlowCalculationTrustLineManager->mvTrustLines.find(mNodeUUID)->second;

        if (sortedTrustLines.size() == 0) {
            return result;
        }
        MaxFlowCalculationTrustLine::Shared &trustLineMax = sortedTrustLines.front();
        auto trustLineFreeAmountPtr = trustLineMax.get()->getFreeAmount();
        if (*trustLineFreeAmountPtr == TrustLine::kZeroAmount()) {
            return result;
        }
        mLog->logInfo("InitiateMaxFlowCalculationTransaction->calculateMaxFlow",
                      "1st max flow: " + to_string((uint32_t)*trustLineFreeAmountPtr));
        trustLineMax = sortedTrustLines.back();
        trustLineFreeAmountPtr = trustLineMax.get()->getFreeAmount();
        mLog->logInfo("InitiateMaxFlowCalculationTransaction->calculateMaxFlow",
                      "1st max flow: " + to_string((uint32_t)*trustLineFreeAmountPtr));
        TrustLineAmount currentFlow = 0;
        for (auto const &trustLine : sortedTrustLines) {
            auto trustLineFreeAmountShared = trustLine.get()->getFreeAmount();
            auto trustLineAmountPtr = trustLineFreeAmountShared.get();
            TrustLineAmount flow = calculateOneNode(trustLine.get()->getTargetUUID(),
                                                    *trustLineAmountPtr, 1, nodeUUID, mNodeUUID);
            if (flow > TrustLine::kZeroAmount()) {
                currentFlow += flow;
                trustLine.get()->addUsedAmount(flow);
                break;
            }
        }
        result += currentFlow;
        if (currentFlow == 0) {
            break;
        }
    }

    return result;
}

TrustLineAmount InitiateMaxFlowCalculationTransaction::calculateOneNode(
    const NodeUUID& nodeUUID,
    const TrustLineAmount& currentFlow,
    int level,
    const NodeUUID& targetUUID,
    const NodeUUID& sourceUUID) {

    if (nodeUUID == targetUUID) {
        return currentFlow;
    }
    if (level == kMaxFlowLength) {
        return 0;
    }
    vector<MaxFlowCalculationTrustLine::Shared> sortedTrustLines =
        mMaxFlowCalculationTrustLineManager->getSortedTrustLines(nodeUUID);
        //mMaxFlowCalculationTrustLineManager->mvTrustLines.find(nodeUUID)->second;
    if (sortedTrustLines.size() == 0) {
        return 0;
    }
    for (auto const &trustLine : sortedTrustLines) {
        if (trustLine.get()->getSourceUUID() == sourceUUID) {
            continue;
        }
        TrustLineAmount nextFlow = currentFlow;
        if (*trustLine.get()->getFreeAmount().get() < currentFlow) {
            nextFlow = *trustLine.get()->getFreeAmount().get();
        }
        if (nextFlow == TrustLine::kZeroAmount()) {
            continue;
        }
        TrustLineAmount calcFlow = calculateOneNode(trustLine.get()->getTargetUUID(),
                                                    nextFlow, level + 1, targetUUID, sourceUUID);
        if (calcFlow > TrustLine::kZeroAmount()) {
            trustLine.get()->addUsedAmount(calcFlow);
            return calcFlow;
        }
    }
    return 0;
}