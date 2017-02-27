//
// Created by mc on 17.02.17.
//

#include "ReceiveResultMaxFlowCalculationTransaction.h"

ReceiveResultMaxFlowCalculationTransaction::ReceiveResultMaxFlowCalculationTransaction(
    NodeUUID &nodeUUID,
    ResultMaxFlowCalculationMessage::Shared message,
    TrustLinesManager *manager,
    MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::ReceiveResultMaxFlowCalculationTransactionType,
        nodeUUID),
    mMessage(message),
    mTrustLinesManager(manager),
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager),
    mLog(logger){}

ReceiveResultMaxFlowCalculationTransaction::ReceiveResultMaxFlowCalculationTransaction(
    BytesShared buffer,
    TrustLinesManager *manager,
    MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager) :

    mTrustLinesManager(manager),
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager){

    deserializeFromBytes(buffer);
}

ResultMaxFlowCalculationMessage::Shared ReceiveResultMaxFlowCalculationTransaction::message() const {

    return mMessage;
}

pair<BytesShared, size_t> ReceiveResultMaxFlowCalculationTransaction::serializeToBytes() const {

    auto parentBytesAndCount = BaseTransaction::serializeToBytes();
    auto messageBytesAndCount = mMessage->serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second + messageBytesAndCount.second;
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    //-----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    //-----------------------------------------------------
    memcpy(
        dataBytesShared.get() + parentBytesAndCount.second,
        messageBytesAndCount.first.get(),
        messageBytesAndCount.second);
    //-----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

void ReceiveResultMaxFlowCalculationTransaction::deserializeFromBytes(
    BytesShared buffer) {

    BaseTransaction::deserializeFromBytes(buffer);
    // todo продумати архітектуру серіалізації - десеріалізаціїї меседжів змінної довжини
    BytesShared messageBufferShared = tryCalloc(ResultMaxFlowCalculationMessage::kRequestedBufferSize(
        buffer.get() + BaseTransaction::kOffsetToInheritedBytes()));
    //-----------------------------------------------------
    memcpy(
        messageBufferShared.get(),
        buffer.get() + BaseTransaction::kOffsetToInheritedBytes(),
        ResultMaxFlowCalculationMessage::kRequestedBufferSize(
            buffer.get() + BaseTransaction::kOffsetToInheritedBytes()));
    //-----------------------------------------------------
    mMessage = ResultMaxFlowCalculationMessage::Shared(
        new ResultMaxFlowCalculationMessage(
            messageBufferShared));
}

TransactionResult::SharedConst ReceiveResultMaxFlowCalculationTransaction::run() {

    mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction->run", "initiator: " + mNodeUUID.stringUUID());
    mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction->run", "sender: " + mMessage->senderUUID().stringUUID());

    mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                  "received trustLines out: " + to_string(mMessage->outgoingFlows().size()));
    for (auto const &outgoingFlow : mMessage->outgoingFlows()) {
        TrustLineAmount trustLineAmount = outgoingFlow.second;
        mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                      outgoingFlow.first.stringUUID() + " " + to_string((uint32_t)trustLineAmount));

        auto trustLine = make_shared<MaxFlowCalculationTrustLine>(
            mMessage->senderUUID(),
            outgoingFlow.first,
            outgoingFlow.second);

        mMaxFlowCalculationTrustLineManager->addTrustLine(trustLine);
        //mMaxFlowCalculationTrustLineManager->addFlow(mMessage->senderUUID(), outgoingFlow.first, outgoingFlow.second);
    }
    mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                  "received trustLines in: " + to_string(mMessage->incomingFlows().size()));
    for (auto const &incomingFlow : mMessage->incomingFlows()) {
        TrustLineAmount trustLineAmount = incomingFlow.second;
        mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                      incomingFlow.first.stringUUID() + " " + to_string((uint32_t)trustLineAmount));

        auto trustLine = make_shared<MaxFlowCalculationTrustLine>(
            incomingFlow.first,
            mMessage->senderUUID(),
            incomingFlow.second);

        mMaxFlowCalculationTrustLineManager->addTrustLine(trustLine);
        //mMaxFlowCalculationTrustLineManager->addFlow(mMessage->senderUUID(), incomingFlow.first, incomingFlow.second);
    }

    mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                  "trustLineMap size: " + to_string(mMaxFlowCalculationTrustLineManager->mvTrustLines.size()));
    for (const auto &nodeUUIDAndTrustLines : mMaxFlowCalculationTrustLineManager->mvTrustLines) {
        mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                      "key: " + ((NodeUUID)nodeUUIDAndTrustLines.first).stringUUID());
        for (const auto &itTrustLine : nodeUUIDAndTrustLines.second) {
            MaxFlowCalculationTrustLine::Shared trustLine = itTrustLine;
            mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                          "value: " + trustLine->getTargetUUID().stringUUID() + " "
                          + to_string((uint32_t)trustLine->getAmount()));
        }
    }

    /*mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                  "entityMap size: " + to_string(mMaxFlowCalculationTrustLineManager->mEntities.size()));
    for (const auto &it : mMaxFlowCalculationTrustLineManager->mEntities) {
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

    if (mMaxFlowCalculationTrustLineManager->mvTrustLines.size() == 6) {
        NodeUUID* globalTargetUUID = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff91");
        NodeUUID targetUUID = *globalTargetUUID;
        TrustLineAmount maxFlow = calculateMaxFlow(targetUUID);
        mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                      "max flow: " + to_string((uint32_t)maxFlow));
    }

    return make_shared<const TransactionResult>(
        TransactionState::exit());

}

TrustLineAmount ReceiveResultMaxFlowCalculationTransaction::calculateMaxFlow(const NodeUUID& nodeUUID) {
    TrustLineAmount result = 0;
    mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction->calculateMaxFlow",
                      "start found flow to: " + nodeUUID.stringUUID());
    while(true) {
        vector<MaxFlowCalculationTrustLine::Shared> sortedTrustLines =
            mMaxFlowCalculationTrustLineManager->mvTrustLines.find(mNodeUUID)->second;

        /*mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction->calculateMaxFlow",
                      "1st max flow size: " + to_string(sortedTrustLines.size()));*/
        if (sortedTrustLines.size() == 0) {
            return result;
        }
        MaxFlowCalculationTrustLine::Shared &trustLineMax = sortedTrustLines.front();
        auto trustLineFreeAmountPtr = trustLineMax.get()->getFreeAmount();
        if (*trustLineFreeAmountPtr == TrustLine::kZeroAmount()) {
            return result;
        }
        /*mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction->calculateMaxFlow",
                      "1st max flow: " + to_string((uint32_t)*trustLineFreeAmountPtr));
        trustLine = sortedTrustLines.back();
        trustLineFreeAmountPtr = trustLine.get()->getFreeAmount();
        mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction->calculateMaxFlow",
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

TrustLineAmount ReceiveResultMaxFlowCalculationTransaction::calculateOneNode(
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
