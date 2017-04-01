#include "CyclesThreeNodesResponseTransaction.h"

CyclesThreeNodesResponseTransaction::CyclesThreeNodesResponseTransaction(TransactionsScheduler *scheduler)
    : UniqueTransaction(BaseTransaction::TransactionType::ThreeNodesResponseTransaction, scheduler) {

}

CyclesThreeNodesResponseTransaction::CyclesThreeNodesResponseTransaction(
    const BaseTransaction::TransactionType type,
    const NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    ThreeNodesBalancesRequestMessage::Shared message,
    TransactionsScheduler *scheduler,
    TrustLinesManager *manager,
    Logger *logger)
    : UniqueTransaction(type, nodeUUID, scheduler),
      mTrustLinesManager(manager),
      mlogger(logger),
      mContractorUUID(contractorUUID),
      mRequestMessage(message) {

}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
TransactionResult::SharedConst CyclesThreeNodesResponseTransaction::run() {
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



