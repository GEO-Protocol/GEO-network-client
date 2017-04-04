#include "CycleThreeNodesResponseTransaction.h"

CycleThreeNodesResponseTransaction::CycleThreeNodesResponseTransaction(TransactionsScheduler *scheduler)
    : UniqueTransaction(BaseTransaction::TransactionType::CycleThreeNodesResponseTransaction, scheduler) {

}

CycleThreeNodesResponseTransaction::CycleThreeNodesResponseTransaction(
    const NodeUUID &nodeUUID,
    ThreeNodesBalancesRequestMessage::Shared message,
    TransactionsScheduler *scheduler,
    TrustLinesManager *manager,
    Logger *logger)
    : UniqueTransaction(BaseTransaction::TransactionType::CycleThreeNodesResponseTransaction, nodeUUID, scheduler),
      mTrustLinesManager(manager),
      mLogger(logger),
      mRequestMessage(message) {

}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
TransactionResult::SharedConst CycleThreeNodesResponseTransaction::run() {
//    Get neighbors UUID from message
    vector<NodeUUID> neighbors = mRequestMessage->Neighbors();
//    Create message and reserve memory for neighbors
    const auto kMessage = make_shared<const ThreeNodesBalancesResponseMessage>(
        mNodeUUID,
        mTransactionUUID,
        neighbors.size()
    );
    vector<pair<NodeUUID, TrustLineBalance>> neighborUUIDAndBalance;
    TrustLineBalance contractorBalance = mTrustLinesManager->balance(mContractorUUID);
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
            neighborUUIDAndBalance.push_back(
                make_pair(
                    value,
                    stepNodeBalance));
    }
    sendMessage<ThreeNodesBalancesResponseMessage>(
        mContractorUUID,
        mNodeUUID,
        mTransactionUUID,
        neighborUUIDAndBalance);
    return finishTransaction();
}
#pragma clang diagnostic pop



