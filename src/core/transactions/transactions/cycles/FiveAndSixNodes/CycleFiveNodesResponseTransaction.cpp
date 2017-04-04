#include "CycleFiveNodesResponseTransaction.h"

CycleFiveNodesResponseTransaction::CycleFiveNodesResponseTransaction(TransactionsScheduler *scheduler)
    : UniqueTransaction(BaseTransaction::TransactionType::CycleFiveNodesResponseTransaction, scheduler) {

}

CycleFiveNodesResponseTransaction::CycleFiveNodesResponseTransaction(
    const NodeUUID &nodeUUID,
    CycleFiveNodesInBetweenMessage::Shared message,
    TransactionsScheduler *scheduler,
    TrustLinesManager *manager,
    Logger *logger)
    : UniqueTransaction(BaseTransaction::TransactionType::CycleFiveNodesResponseTransaction, nodeUUID, scheduler),
      mTrustLinesManager(manager),
      mlogger(logger),
      mInBetweenNodeTopologyMessage(message) {

}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
TransactionResult::SharedConst CycleFiveNodesResponseTransaction::run() {
    vector<NodeUUID> path = mInBetweenNodeTopologyMessage->Path();
    uint8_t currentDepth = path.size();
    TrustLineBalance zeroBalance = 0;
    TrustLineBalance maxFlow = mTrustLinesManager->balance(path.back());
    vector<pair<NodeUUID, TrustLineBalance>> firstLevelNodes = mTrustLinesManager->getFirstLevelNodesForCycles(maxFlow);

    bool debtorsDirection = true;
    if (maxFlow < zeroBalance){
        debtorsDirection = false;
    }
    if ((debtorsDirection and currentDepth==1)) {
        mInBetweenNodeTopologyMessage->addNodeToPath(mNodeUUID);
        for(const auto &value: firstLevelNodes)
            sendMessage(
                value.first,
                mInBetweenNodeTopologyMessage
            );
        return resultExit();
    }
    if ((debtorsDirection and currentDepth==2) or (not debtorsDirection and currentDepth==1)){
        path.push_back(mNodeUUID);
        sendMessage<CycleFiveNodesBoundaryMessage>(
            path.front(),
            path,
            firstLevelNodes
        );
        return resultExit();
    }
    else {
        return resultExit();
    }

}
#pragma clang diagnostic pop

