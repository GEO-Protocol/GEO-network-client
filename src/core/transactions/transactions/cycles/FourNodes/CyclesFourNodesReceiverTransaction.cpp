#include "CyclesFourNodesReceiverTransaction.h"

CyclesFourNodesReceiverTransaction::CyclesFourNodesReceiverTransaction(
    const NodeUUID &nodeUUID,
    CyclesFourNodesBalancesRequestMessage::Shared message,
    TrustLinesManager *manager,
    Logger *logger) :
    BaseTransaction(
        BaseTransaction::TransactionType::Cycles_FourNodesReceiverTransaction,
        message->transactionUUID(),
        nodeUUID,
        logger),
    mTrustLinesManager(manager),
    mRequestMessage(message)
{}

TransactionResult::SharedConst CyclesFourNodesReceiverTransaction::run() {
    const auto kNeighbors = mRequestMessage->Neighbors();
    const auto kMessage = make_shared<CyclesFourNodesBalancesResponseMessage>(
        mNodeUUID,
        currentTransactionUUID(),
        kNeighbors.size());

    const auto kContractorBalance = mTrustLinesManager->balance(mRequestMessage->senderUUID);
    const TrustLineBalance kZeroBalance = 0;
    TrustLineBalance stepNodeBalance;

    bool searchDebtors = true;
    if (kContractorBalance > kZeroBalance)
        searchDebtors = false;

    for (auto &kNodeUUID: kNeighbors) {
        stepNodeBalance = mTrustLinesManager->balance(kNodeUUID);
        if ((searchDebtors and (stepNodeBalance > kZeroBalance)) or
            (not searchDebtors and (stepNodeBalance < kZeroBalance)))
            kMessage->AddNeighborUUID(kNodeUUID);
    }
    if (kMessage->NeighborsUUID().size() > 0)
        sendMessage(mRequestMessage->senderUUID, kMessage);
    return resultDone();
}

const string CyclesFourNodesReceiverTransaction::logHeader() const
{
    stringstream s;
    s << "[CyclesFourNodesReceiverTransactionTA: " << currentTransactionUUID() << "] ";

    return s.str();
}