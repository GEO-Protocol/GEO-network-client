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

InitiateMaxFlowCalculationTransaction::InitiateMaxFlowCalculationTransaction(
        BytesShared buffer,
        TrustLinesManager *manager,
        MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager) :

    mTrustLinesManager(manager),
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager){

    deserializeFromBytes(buffer);
}

InitiateMaxFlowCalculationCommand::Shared InitiateMaxFlowCalculationTransaction::command() const {

    return mCommand;
}

pair<BytesShared, size_t> InitiateMaxFlowCalculationTransaction::serializeToBytes() const {

    auto parentBytesAndCount = BaseTransaction::serializeToBytes();
    auto commandBytesAndCount = mCommand->serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second +  commandBytesAndCount.second;
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    //-----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second
    );
    //-----------------------------------------------------
    memcpy(
        dataBytesShared.get() + parentBytesAndCount.second,
        commandBytesAndCount.first.get(),
        commandBytesAndCount.second
    );
    //-----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount
    );
}

void InitiateMaxFlowCalculationTransaction::deserializeFromBytes(
        BytesShared buffer) {

    BaseTransaction::deserializeFromBytes(buffer);
    BytesShared commandBufferShared = tryCalloc(InitiateMaxFlowCalculationCommand::kRequestedBufferSize());
    //-----------------------------------------------------
    memcpy(
            commandBufferShared.get(),
            buffer.get() + BaseTransaction::kOffsetToInheritedBytes(),
            InitiateMaxFlowCalculationCommand::kRequestedBufferSize());
    //-----------------------------------------------------
    mCommand = InitiateMaxFlowCalculationCommand::Shared(
            new InitiateMaxFlowCalculationCommand(
                    commandBufferShared));
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

    sendMessageToRemoteNode();
    sendMessageOnFirstLevel();

    TrustLineAmount maxFlow = calculateMaxFlow(mCommand->contractorUUID());
    mLog->logInfo("InitiateMaxFlowCalculationTransaction::run",
                  "max flow: " + to_string((uint32_t)maxFlow));

    return make_shared<TransactionResult>(TransactionState::exit());

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
            mMaxFlowCalculationTrustLineManager->mvTrustLines.find(mNodeUUID)->second;

        MaxFlowCalculationTrustLine::Shared &trustLineA = sortedTrustLines.front();
        MaxFlowCalculationTrustLine::Shared &trustLineB = sortedTrustLines.back();
        testCompare(trustLineA, trustLineB);
        /*mLog->logInfo("InitiateMaxFlowCalculationTransaction->calculateMaxFlow",
                      "1st max flow size: " + to_string(sortedTrustLines.size()));*/
        if (sortedTrustLines.size() == 0) {
            return result;
        }
        MaxFlowCalculationTrustLine::Shared &trustLineMax = sortedTrustLines.front();
        auto trustLineFreeAmountPtr = trustLineMax.get()->getFreeAmount();
        if (*trustLineFreeAmountPtr == TrustLine::kZeroAmount()) {
            return result;
        }
        /*mLog->logInfo("InitiateMaxFlowCalculationTransaction->calculateMaxFlow",
                      "1st max flow: " + to_string((uint32_t)*trustLineFreeAmountPtr));
        trustLine = sortedTrustLines.back();
        trustLineFreeAmountPtr = trustLine.get()->getFreeAmount();
        mLog->logInfo("InitiateMaxFlowCalculationTransaction->calculateMaxFlow",
                      "1st max flow: " + to_string((uint32_t)*trustLineFreeAmountPtr));*/
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
    if (level == 6) {
        return 0;
    }
    vector<MaxFlowCalculationTrustLine::Shared> sortedTrustLines =
        mMaxFlowCalculationTrustLineManager->mvTrustLines.find(nodeUUID)->second;
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

void InitiateMaxFlowCalculationTransaction::testCompare(
    MaxFlowCalculationTrustLine::Shared a,
    MaxFlowCalculationTrustLine::Shared b) {
    auto aTrustLineFreeAmountPtr = a.get()->getFreeAmount();
    auto bTrustLineFreeAmountPtr = b.get()->getFreeAmount();
    cout << *aTrustLineFreeAmountPtr << " " << *bTrustLineFreeAmountPtr << "\n";
}