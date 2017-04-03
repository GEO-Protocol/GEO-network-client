#include "CycleThreeNodesResponseTransaction.h"

CycleThreeNodesResponseTransaction::CycleThreeNodesResponseTransaction(TransactionsScheduler *scheduler)
    : UniqueTransaction(BaseTransaction::TransactionType::CycleThreeNodesResponseTransaction, scheduler) {

}

CycleThreeNodesResponseTransaction::CycleThreeNodesResponseTransaction(
    const BaseTransaction::TransactionType type,
    const NodeUUID &nodeUUID,
    ThreeNodesBalancesRequestMessage::Shared message,
    TransactionsScheduler *scheduler,
    TrustLinesManager *manager,
    Logger *logger)
    : UniqueTransaction(type, nodeUUID, scheduler),
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
    TrustLineBalance contractorBalance = mTrustLinesManager->balance(mContractorUUID);
    TrustLineBalance zeroBalance = 0;
    TrustLineBalance stepNodeBalance;
    bool searchDebtors = true;
    if (contractorBalance > zeroBalance)
        searchDebtors = false;

    for (auto &value: neighbors) {
        stepNodeBalance = mTrustLinesManager->balance(value);
        if (searchDebtors and (stepNodeBalance > zeroBalance))
            kMessage->AddNeighborUUIDAndBalance(
                make_pair(
                    value,
                    stepNodeBalance));
        if (not searchDebtors and (stepNodeBalance < zeroBalance))
            kMessage->AddNeighborUUIDAndBalance(
                make_pair(
                    value,
                    stepNodeBalance));
    }
    sendMessage(mContractorUUID, kMessage);
    return finishTransaction();
}
#pragma clang diagnostic pop



