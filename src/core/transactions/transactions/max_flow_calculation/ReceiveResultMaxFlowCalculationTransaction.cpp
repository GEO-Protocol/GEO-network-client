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

ResultMaxFlowCalculationMessage::Shared ReceiveResultMaxFlowCalculationTransaction::message() const {

    return mMessage;
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

    return make_shared<const TransactionResult>(
        TransactionState::exit());

}
