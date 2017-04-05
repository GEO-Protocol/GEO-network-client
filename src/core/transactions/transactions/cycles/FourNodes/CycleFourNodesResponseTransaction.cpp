#include "CycleFourNodesResponseTransaction.h"

CycleFourNodesResponseTransaction::CycleFourNodesResponseTransaction(TransactionsScheduler *scheduler)
    : UniqueTransaction(BaseTransaction::TransactionType::CycleFourNodesResponseTransaction, scheduler) {

}

CycleFourNodesResponseTransaction::CycleFourNodesResponseTransaction(
    const NodeUUID &nodeUUID,
    FourNodesBalancesRequestMessage::Shared message,
    TransactionsScheduler *scheduler,
    TrustLinesManager *manager,
    Logger *logger)
    : UniqueTransaction(BaseTransaction::TransactionType::CycleFourNodesResponseTransaction, nodeUUID, scheduler),
      mTrustLinesManager(manager),
      mLogger(logger),
      mRequestMessage(message) {

}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
TransactionResult::SharedConst CycleFourNodesResponseTransaction::run() {
//    Get neighbors UUID from message
    set<NodeUUID> neighbors = mRequestMessage->Neighbors();
//    Create message and reserve memory for neighbors
    const auto kMessage = make_shared<FourNodesBalancesResponseMessage>(
        mNodeUUID,
        mTransactionUUID,
        neighbors.size()
    );
    vector<pair<NodeUUID, TrustLineBalance>> neighborUUIDAndBalance;
    TrustLineBalance contractorBalance = mTrustLinesManager->balance(mRequestMessage->senderUUID());
    TrustLineBalance zeroBalance = 0;
    TrustLineBalance stepNodeBalance;
    bool searchDebtors = true;
    if (contractorBalance < zeroBalance)
        searchDebtors = false;
//    todo Add resize to AddNeighborUUIDAndBalance
    for (auto &value: neighbors) {
        stepNodeBalance = mTrustLinesManager->balance(value);
        if ((searchDebtors and (stepNodeBalance > zeroBalance)) or
            (not searchDebtors and (stepNodeBalance < zeroBalance)))
            kMessage->AddNeighborUUIDAndBalance(
                make_pair(
                    value,
                    stepNodeBalance));
    }
    sendMessage(mRequestMessage->senderUUID(), kMessage);
    return finishTransaction();
}
#pragma clang diagnostic pop



