#include "CycleFiveNodesResponseTransaction.h"

CycleFiveNodesResponseTransaction::CycleFiveNodesResponseTransaction(TransactionsScheduler *scheduler)
    : UniqueTransaction(BaseTransaction::TransactionType::CycleFiveNodesResponseTransaction, scheduler) {

}

CycleFiveNodesResponseTransaction::CycleFiveNodesResponseTransaction(
    const BaseTransaction::TransactionType type,
    const NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    CycleFiveNodesInBetweenMessage::Shared message,
    TransactionsScheduler *scheduler,
    TrustLinesManager *manager,
    Logger *logger)
    : UniqueTransaction(type, nodeUUID, scheduler),
      mTrustLinesManager(manager),
      mlogger(logger),
      mContractorUUID(contractorUUID),
      mInBetweenNodeTopologyMessage(message) {

}

TransactionResult::SharedConst CycleFiveNodesResponseTransaction::run() {
    vector<NodeUUID> path = mInBetweenNodeTopologyMessage->Path();
    TrustLineBalance zeroBalance = 0;
    TrustLineBalance maxFlow = mTrustLinesManager->balance(path.front());
    vector<pair<NodeUUID, TrustLineBalance>> firstLevelNodes = mTrustLinesManager->getFirstLevelNodesForCycles(maxFlow);

    bool debtorsDirection = true;
    if (maxFlow < zeroBalance){
        positiveDirection = false;
    }

    if (path.size() == mInBetweenNodeTopologyMessage->maxDepth()) {
        path.push_back(mNodeUUID);
        sendMessage<CycleFiveNodesBoundaryMessage>(
            mInBetweenNodeTopologyMessage->Path().front(),
            path,
            firstLevelNodes
        );
        return resultExit();
    } else if (path.size() < mInBetweenNodeTopologyMessage->maxDepth()){
        mInBetweenNodeTopologyMessage->addNodeToPath(mNodeUUID);
        for(const auto &value: firstLevelNodes)
            sendMessage(
                value.first,
                mInBetweenNodeTopologyMessage
            );
        return resultExit();
    }
    return resultExit();
}

