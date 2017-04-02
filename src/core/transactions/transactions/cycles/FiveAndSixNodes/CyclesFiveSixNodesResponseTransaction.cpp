#include "CyclesFiveSixNodesResponseTransaction.h"

CyclesFiveSixNodesResponseTransaction::CyclesFiveSixNodesResponseTransaction(TransactionsScheduler *scheduler)
    : UniqueTransaction(BaseTransaction::TransactionType::CyclesFiveSixNodesResponseTransaction, scheduler) {

}

CyclesFiveSixNodesResponseTransaction::CyclesFiveSixNodesResponseTransaction(
    const BaseTransaction::TransactionType type,
    const NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    InBetweenNodeTopologyMessage::Shared message,
    TransactionsScheduler *scheduler,
    TrustLinesManager *manager,
    Logger *logger)
    : UniqueTransaction(type, nodeUUID, scheduler),
      mTrustLinesManager(manager),
      mlogger(logger),
      mContractorUUID(contractorUUID),
      mInBetweenNodeTopologyMessage(message) {

}

TransactionResult::SharedConst CyclesFiveSixNodesResponseTransaction::run() {
    vector<NodeUUID> path = mInBetweenNodeTopologyMessage->Path();
    TrustLineBalance maxFlow = mTrustLinesManager->balance(path.front());
    vector<pair<NodeUUID, TrustLineBalance>> firstLevelNodes = mTrustLinesManager->getFirstLevelNodesForCycles(maxFlow);
    if (path.size() == mInBetweenNodeTopologyMessage->maxDepth()) {
        path.push_back(mNodeUUID);
        sendMessage<BoundaryNodeTopologyMessage>(
            mInBetweenNodeTopologyMessage->Path().front(),
            mInBetweenNodeTopologyMessage->cycleType(),
            mInBetweenNodeTopologyMessage->maxDepth(),
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

