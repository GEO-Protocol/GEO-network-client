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

    mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run", "initiator: " + mNodeUUID.stringUUID());
    mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run", "sender: " + mMessage->senderUUID().stringUUID());

//#ifdef TESTS
    uint32_t countTrustLinesBeforeInsert = 0;
    for (const auto &nodeUUIDAndTrustLines : mMaxFlowCalculationTrustLineManager->msTrustLines) {
        countTrustLinesBeforeInsert += (nodeUUIDAndTrustLines.second)->size();
    }
    mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run", "beforeInsert mapTrustLinesCount: " + to_string(countTrustLinesBeforeInsert));
//#endif

    mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                  "receivedTrustLinesOut: " + to_string(mMessage->outgoingFlows().size()));
    for (auto const &outgoingFlow : mMessage->outgoingFlows()) {
        TrustLineAmount trustLineAmount = outgoingFlow.second;
        mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                      outgoingFlow.first.stringUUID() + " " + to_string((uint32_t)trustLineAmount));


        auto trustLine = make_shared<MaxFlowCalculationTrustLine>(
            mMessage->senderUUID(),
            outgoingFlow.first,
            outgoingFlow.second);

        mMaxFlowCalculationTrustLineManager->addTrustLine(trustLine);
    }
    mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                  "receivedTrustLinesIn: " + to_string(mMessage->incomingFlows().size()));
    for (auto const &incomingFlow : mMessage->incomingFlows()) {
        TrustLineAmount trustLineAmount = incomingFlow.second;
        mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                      incomingFlow.first.stringUUID() + " " + to_string((uint32_t)trustLineAmount));

        auto trustLine = make_shared<MaxFlowCalculationTrustLine>(
            incomingFlow.first,
            mMessage->senderUUID(),
            incomingFlow.second);

        mMaxFlowCalculationTrustLineManager->addTrustLine(trustLine);
    }

//#ifdef TESTS
    uint32_t countTrustLinesAfterInsert = 0;
    for (const auto &nodeUUIDAndTrustLines : mMaxFlowCalculationTrustLineManager->msTrustLines) {
        countTrustLinesAfterInsert += (nodeUUIDAndTrustLines.second)->size();
    }
    mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run", "afterInsert mapTrustLinesCount: " + to_string(countTrustLinesAfterInsert));
//#endif

    mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                  "trustLineMap size: " + to_string(mMaxFlowCalculationTrustLineManager->msTrustLines.size()));
    for (const auto &nodeUUIDAndTrustLines : mMaxFlowCalculationTrustLineManager->msTrustLines) {
        mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                      "key: " + ((NodeUUID)nodeUUIDAndTrustLines.first).stringUUID());
        for (auto &itTrustLine : *nodeUUIDAndTrustLines.second) {
            MaxFlowCalculationTrustLine::Shared trustLine = itTrustLine->maxFlowCalculationtrustLine();
            mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                          "value: " + trustLine->targetUUID().stringUUID() + " "
                          + to_string((uint32_t) trustLine->amount()));
        }
    }

    return make_shared<const TransactionResult>(
        TransactionState::exit());

}
