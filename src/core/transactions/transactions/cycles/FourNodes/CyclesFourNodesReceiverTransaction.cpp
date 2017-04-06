#include "CyclesFourNodesReceiverTransaction.h"

CyclesFourNodesReceiverTransaction::CyclesFourNodesReceiverTransaction(
    const NodeUUID &nodeUUID,
    CyclesFourNodesBalancesRequestMessage::Shared message,
    TrustLinesManager *manager,
    Logger *logger) :
    BaseTransaction(
        BaseTransaction::TransactionType::Cycles_FourNodesReceiverTransaction,
        nodeUUID),
    mTrustLinesManager(manager),
    mLogger(logger),
    mRequestMessage(message)
{}

TransactionResult::SharedConst CyclesFourNodesReceiverTransaction::run() {
    const auto kNeighbors = mRequestMessage->Neighbors();
    const auto kMessage = make_shared<CyclesFourNodesBalancesResponseMessage>(
        mNodeUUID,
        mTransactionUUID,
        kNeighbors.size());

    const auto kContractorBalance = mTrustLinesManager->balance(mRequestMessage->senderUUID());
    const TrustLineBalance kZeroBalance = 0;
    TrustLineBalance stepNodeBalance;

    bool searchDebtors = true;
    if (kContractorBalance < kZeroBalance)
        searchDebtors = false;

    for (auto &kNodeUUID: kNeighbors) {
        stepNodeBalance = mTrustLinesManager->balance(kNodeUUID);
        if ((searchDebtors and (stepNodeBalance > kZeroBalance)) or
            (not searchDebtors and (stepNodeBalance < kZeroBalance)))
            kMessage->AddNeighborUUIDAndBalance(
                make_pair(
                    kNodeUUID,
                    stepNodeBalance));
    }

    sendMessage(mRequestMessage->senderUUID(), kMessage);
    return finishTransaction();
}