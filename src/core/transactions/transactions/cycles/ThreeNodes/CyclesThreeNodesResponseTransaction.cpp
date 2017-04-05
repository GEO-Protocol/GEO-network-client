#include "CyclesThreeNodesResponseTransaction.h"

CyclesThreeNodesResponseTransaction::CyclesThreeNodesResponseTransaction(
    const NodeUUID &nodeUUID,
    ThreeNodesBalancesRequestMessage::Shared message,
    TrustLinesManager *manager,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::Cycles_ThreeNodesResponseTransaction,
        nodeUUID),
    mTrustLinesManager(manager),
    mLogger(logger),
    mRequestMessage(message) {

}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
TransactionResult::SharedConst CyclesThreeNodesResponseTransaction::run() {
//    Get neighbors UUID from message
    vector<NodeUUID> neighbors = mRequestMessage->Neighbors();
//    Create message and reserve memory for neighbors
    const auto kMessage = make_shared<ThreeNodesBalancesResponseMessage>(
        mNodeUUID,
        mTransactionUUID,
        neighbors.size()
    );
    vector<pair<NodeUUID, TrustLineBalance>> neighborUUIDAndBalance;
    const auto contractorBalance = mTrustLinesManager->balance(mRequestMessage->senderUUID());
    TrustLineBalance zeroBalance = 0;
    TrustLineBalance stepNodeBalance;
    bool searchDebtors = true;
    if (contractorBalance < zeroBalance)
        searchDebtors = false;
    for (auto &nodeUUID: neighbors) {
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
#pragma clang diagnostic pop



