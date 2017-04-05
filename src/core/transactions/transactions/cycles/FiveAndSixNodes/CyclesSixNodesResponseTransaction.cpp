#include "CyclesSixNodesResponseTransaction.h"

CyclesSixNodesResponseTransaction::CyclesSixNodesResponseTransaction(TransactionsScheduler *scheduler)
    : UniqueTransaction(BaseTransaction::TransactionType::Cycles_SixNodesResponseTransaction, scheduler) {

}

CyclesSixNodesResponseTransaction::CyclesSixNodesResponseTransaction(
    const NodeUUID &nodeUUID,
    CycleSixNodesInBetweenMessage::Shared message,
    TransactionsScheduler *scheduler,
    TrustLinesManager *manager,
    Logger *logger)
    : UniqueTransaction(BaseTransaction::TransactionType::Cycles_SixNodesResponseTransaction, nodeUUID, scheduler),
      mTrustLinesManager(manager),
      mlogger(logger),
      mInBetweenNodeTopologyMessage(message) {
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
TransactionResult::SharedConst CyclesSixNodesResponseTransaction::run() {
    vector<NodeUUID> path = mInBetweenNodeTopologyMessage->Path();
    uint8_t currentDepth = path.size();
    TrustLineBalance zeroBalance = 0;
    TrustLineBalance maxFlow = mTrustLinesManager->balance(path.back());

//  If balance to previous node equal zero finish transaction
    if (maxFlow == zeroBalance)
        return resultExit();
    vector<pair<NodeUUID, TrustLineBalance>> firstLevelNodes = mTrustLinesManager->getFirstLevelNodesForCycles(maxFlow);
//  Update message path and send to next level nodes
    if ((currentDepth==1)) {
        mInBetweenNodeTopologyMessage->addNodeToPath(mNodeUUID);
        for(const auto &value: firstLevelNodes)
            sendMessage(
                value.first,
                mInBetweenNodeTopologyMessage
            );
    }
    if (currentDepth==2){
        path.push_back(mNodeUUID);
        sendMessage<CycleFiveNodesBoundaryMessage>(
            path.front(),
            path,
            firstLevelNodes
        );
    }
    else {
        mlogger->error("CyclesSixNodesResponseTransaction:"
        "Wrong path size");
    }
    return resultExit();
}