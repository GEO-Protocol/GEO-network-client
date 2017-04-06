#include "CyclesFourNodesResponseTransaction.h"

CyclesFourNodesResponseTransaction::CyclesFourNodesResponseTransaction(
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

TransactionResult::SharedConst CyclesFourNodesResponseTransaction::run() {
    const auto kNeighbors = mRequestMessage->Neighbors();
    const auto kMessage = make_shared<CyclesFourNodesBalancesResponseMessage>(
        mNodeUUID,
        mTransactionUUID,
        kNeighbors.size());

    const auto contractorBalance = mTrustLinesManager->balance(mRequestMessage->senderUUID());
    TrustLineBalance zeroBalance = 0;
    TrustLineBalance stepNodeBalance;

    bool searchDebtors = true;
    if (contractorBalance < zeroBalance)
        searchDebtors = false;

    for (auto &nodeUUID: kNeighbors) {
        stepNodeBalance = mTrustLinesManager->balance(nodeUUID);
        if ((searchDebtors and (stepNodeBalance > zeroBalance)) or
            (not searchDebtors and (stepNodeBalance < zeroBalance)))
            kMessage->AddNeighborUUIDAndBalance(
                make_pair(
                    nodeUUID,
                    stepNodeBalance));
    }

    sendMessage(mRequestMessage->senderUUID(), kMessage);
    return finishTransaction();
}