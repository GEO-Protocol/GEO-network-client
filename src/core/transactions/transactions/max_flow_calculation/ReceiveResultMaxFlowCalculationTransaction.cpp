#include "ReceiveResultMaxFlowCalculationTransaction.h"

ReceiveResultMaxFlowCalculationTransaction::ReceiveResultMaxFlowCalculationTransaction(
    NodeUUID &nodeUUID,
    ResultMaxFlowCalculationMessage::Shared message,
    TrustLinesManager *manager,
    MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::ReceiveResultMaxFlowCalculationTransactionType,
        nodeUUID,
        logger),
    mMessage(message),
    mTrustLinesManager(manager),
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager) {}

ResultMaxFlowCalculationMessage::Shared ReceiveResultMaxFlowCalculationTransaction::message() const {

    return mMessage;
}

TransactionResult::SharedConst ReceiveResultMaxFlowCalculationTransaction::run() {

    info() << "run\t" << "initiator: " << mNodeUUID.stringUUID();
    info() << "run\t" << "sender: " << mMessage->senderUUID().stringUUID();

#ifdef TESTS
    uint32_t countTrustLinesBeforeInsert = 0;
    for (const auto &nodeUUIDAndTrustLines : mMaxFlowCalculationTrustLineManager->msTrustLines) {
        countTrustLinesBeforeInsert += (nodeUUIDAndTrustLines.second)->size();
    }
    info() << "run\t" << "beforeInsert mapTrustLinesCount: " << countTrustLinesBeforeInsert;
#endif

    info() << "run\t" << "receivedTrustLinesOut: " << mMessage->outgoingFlows().size();
    for (auto const &outgoingFlow : mMessage->outgoingFlows()) {
        TrustLineAmount trustLineAmount = outgoingFlow.second;
        info() << "run\t" << outgoingFlow.first.stringUUID() << " " << trustLineAmount;


        auto trustLine = make_shared<MaxFlowCalculationTrustLine>(
            mMessage->senderUUID(),
            outgoingFlow.first,
            outgoingFlow.second);

        mMaxFlowCalculationTrustLineManager->addTrustLine(trustLine);
    }
    info() << "run\t" << "receivedTrustLinesIn: " << mMessage->incomingFlows().size();
    for (auto const &incomingFlow : mMessage->incomingFlows()) {
        TrustLineAmount trustLineAmount = incomingFlow.second;
        info() << "run\t" << incomingFlow.first.stringUUID() << " " << trustLineAmount;

        auto trustLine = make_shared<MaxFlowCalculationTrustLine>(
            incomingFlow.first,
            mMessage->senderUUID(),
            incomingFlow.second);

        mMaxFlowCalculationTrustLineManager->addTrustLine(trustLine);
    }

#ifdef TESTS
    uint32_t countTrustLinesAfterInsert = 0;
    for (const auto &nodeUUIDAndTrustLines : mMaxFlowCalculationTrustLineManager->msTrustLines) {
        countTrustLinesAfterInsert += (nodeUUIDAndTrustLines.second)->size();
    }
    info() << "run\t" << "afterInsert mapTrustLinesCount: " << countTrustLinesAfterInsert;
#endif

    info() << "run\t" << "trustLineMap size: " << mMaxFlowCalculationTrustLineManager->msTrustLines.size();
    for (const auto &nodeUUIDAndTrustLines : mMaxFlowCalculationTrustLineManager->msTrustLines) {
        info() << "run\t" << "key: " << nodeUUIDAndTrustLines.first.stringUUID();
        for (auto &itTrustLine : *nodeUUIDAndTrustLines.second) {
            MaxFlowCalculationTrustLine::Shared trustLine = itTrustLine->maxFlowCalculationtrustLine();
            info() << "run\t" << "value: " << trustLine->targetUUID().stringUUID() << " " << trustLine->amount();
        }
    }

    return make_shared<const TransactionResult>(
        TransactionState::exit());
}

const string ReceiveResultMaxFlowCalculationTransaction::logHeader() const
{
    stringstream s;
    s << "[ReceiveResultMaxFlowCalculationTA]";

    return s.str();
}

