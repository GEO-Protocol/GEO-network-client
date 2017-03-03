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
                nodeUUID
        ),
        mCommand(command),
        mTrustLinesManager(manager),
        mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager),
        mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager),
        mLog(logger){}

InitiateMaxFlowCalculationCommand::Shared InitiateMaxFlowCalculationTransaction::command() const {

    return mCommand;
}

TransactionResult::SharedConst InitiateMaxFlowCalculationTransaction::run() {

    mLog->logInfo("InitiateMaxFlowCalculationTransaction->run", "initiator: " + mNodeUUID.stringUUID());
    mLog->logInfo("InitiateMaxFlowCalculationTransaction->run", "target: " + mCommand->contractorUUID().stringUUID());
    /*mLog->logInfo("InitiateMaxFlowCalculationTransaction->run",
                  "OutgoingFlows: " + to_string(mTrustLinesManager->outgoingFlows().size()));
    for (auto const &nodeUUIDAndTrustLine : mTrustLinesManager->outgoingFlows()) {
        {
            auto info = mLog->info("InitiateMaxFlowCalculationTransaction->run");
            info << "out flow " << nodeUUIDAndTrustLine.first.stringUUID() << "  " <<  nodeUUIDAndTrustLine.second << "\n";
        }
    }

    mLog->logInfo("InitiateMaxFlowCalculationTransaction->run",
                  "IncomingFlows: " + to_string(mTrustLinesManager->getIncomingFlows().size()));
    for (auto const &nodeUUIDAndTrustLine : mTrustLinesManager->incomingFlows()) {
        mLog->logInfo("InitiateMaxFlowCalculationTransaction->run", "in flow: " + nodeUUIDAndTrustLine.first.stringUUID()
                                                                    + "  " + to_string((uint32_t)nodeUUIDAndTrustLine.second));
    }*/


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

    /*mMaxFlowCalculationCacheManager->testSet();
    return make_shared<TransactionResult>(TransactionState::exit());*/

    /*mMaxFlowCalculationCacheManager->testMap();
    return make_shared<TransactionResult>(TransactionState::exit());*/

    if (mStep == 1) {
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

            mLog->logInfo("InitiateMaxFlowCalculationTransaction::run", "step " + to_string(mStep));
            /*TrustLineAmount maxFlow = calculateMaxFlow(mCommand->contractorUUID());
            mLog->logInfo("InitiateMaxFlowCalculationTransaction::run",
                          "max flow: " + to_string((uint32_t)maxFlow));*/
        }
        sendMessageToRemoteNode();
        increaseStepsCounter();
        return make_shared<TransactionResult>(
            TransactionState::awakeAfterMilliseconds(kWaitMilisecondsForCalculatingMaxFlow));
    } else {
        TrustLineAmount maxFlow = calculateMaxFlow(mCommand->contractorUUID());
        mLog->logInfo("InitiateMaxFlowCalculationTransaction::run",
                      "max flow: " + to_string((uint32_t)maxFlow));
        resetStepsCounter();
        //return resultOk(maxFlow);
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

void InitiateMaxFlowCalculationTransaction::sendMessagesOnFirstLevel() {

    vector<NodeUUID> outgoingFlowUuids = mTrustLinesManager->firstLevelNeighborsWithOutgoingFlow();
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
            mMaxFlowCalculationTrustLineManager->sortedTrustLines(mNodeUUID);

        if (sortedTrustLines.size() == 0) {
            return result;
        }
        MaxFlowCalculationTrustLine::Shared &trustLineMax = *sortedTrustLines.begin();
        auto trustLineFreeAmountPtr = trustLineMax.get()->freeAmount();
        if (*trustLineFreeAmountPtr == TrustLine::kZeroAmount()) {
            return result;
        }
        mLog->logInfo("InitiateMaxFlowCalculationTransaction->calculateMaxFlow",
                      "1st max flow: " + to_string((uint32_t)*trustLineFreeAmountPtr));

        TrustLineAmount currentFlow = 0;
        for (auto &trustLine : sortedTrustLines) {
            auto trustLineFreeAmountShared = trustLine.get()->freeAmount();
            auto trustLineAmountPtr = trustLineFreeAmountShared.get();
            /*mLog->logInfo("InitiateMaxFlowCalculationTransaction->calculateMaxFlow",
                          "go in: " + trustLine.get()->sourceUUID().stringUUID() + "->"
                          + trustLine.get()->targetUUID().stringUUID() + " " + to_string((uint32_t)*trustLineAmountPtr));*/
            set<NodeUUID> forbiddenUUIDs;
            TrustLineAmount flow = calculateOneNode(
                trustLine.get()->targetUUID(),
                *trustLineAmountPtr,
                1,
                nodeUUID,
                mNodeUUID,
                forbiddenUUIDs);
            if (flow > TrustLine::kZeroAmount()) {
                mLog->logInfo("InitiateMaxFlowCalculationTransaction->calculateMaxFlow",
                              "used flow: " + trustLine.get()->sourceUUID().stringUUID() + "->"
                              + trustLine.get()->targetUUID().stringUUID() + " " + to_string((uint32_t)flow));
                currentFlow += flow;
                trustLine->addUsedAmount(flow);
                auto trustLineFreeAmountTmp = trustLine.get()->freeAmount();
                auto trustLineAmountTmp = trustLineFreeAmountTmp.get();
                mLog->logInfo("InitiateMaxFlowCalculationTransaction->calculateMaxFlow",
                              "new flow: " + to_string((uint32_t)*trustLineAmountTmp));
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

    mLog->logInfo("InitiateMaxFlowCalculationTransaction->calculateMaxFlow",
                  "go in: " + nodeUUID.stringUUID() + "->"
                  + to_string((uint32_t)currentFlow) + "->" + to_string(level));
    mLog->logInfo("InitiateMaxFlowCalculationTransaction->calculateMaxFlow",
                  "forbidden nodes: " + to_string(forbiddenNodeUUIDs.size()));
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
            mLog->logInfo("InitiateMaxFlowCalculationTransaction->calculateMaxFlow",
                          "used flow: " + trustLine.get()->sourceUUID().stringUUID() + "->"
                          + trustLine.get()->targetUUID().stringUUID() + " " + to_string((uint32_t)calcFlow));
            trustLine->addUsedAmount(calcFlow);
            auto trustLineFreeAmountTmp = trustLine.get()->freeAmount();
            auto trustLineAmountTmp = trustLineFreeAmountTmp.get();
            mLog->logInfo("InitiateMaxFlowCalculationTransaction->calculateMaxFlow",
                          "new flow: " + to_string((uint32_t)*trustLineAmountTmp));
            return calcFlow;
        }
    }
    return 0;
}

TransactionResult::SharedConst InitiateMaxFlowCalculationTransaction::resultOk(TrustLineAmount &maxFlowAmount) {

    string maxFlowAmountStr = to_string((uint32_t)maxFlowAmount);
    return transactionResultFromCommand(mCommand->resultOk(maxFlowAmountStr));
}