#include "CyclesSixNodesResponseTransaction.h"

CyclesSixNodesResponseTransaction::CyclesSixNodesResponseTransaction(
    const NodeUUID &nodeUUID,
    CycleSixNodesInBetweenMessage::Shared message,
    TrustLinesManager *manager,
    Logger *logger) :
    BaseTransaction(
        BaseTransaction::TransactionType::Cycles_SixNodesResponseTransaction,
        nodeUUID),
    mTrustLinesManager(manager),
    mLogger(logger),
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
        mLogger->error("CyclesSixNodesResponseTransaction:"
        "Wrong path size");
    }
    return resultExit();
}