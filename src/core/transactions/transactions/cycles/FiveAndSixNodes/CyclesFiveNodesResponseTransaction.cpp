#include "CyclesFiveNodesResponseTransaction.h"

CyclesFiveNodesResponseTransaction::CyclesFiveNodesResponseTransaction(
    const NodeUUID &nodeUUID,
    CycleFiveNodesInBetweenMessage::Shared message,
    TrustLinesManager *manager,
    Logger *logger) :
    BaseTransaction(
        BaseTransaction::TransactionType::Cycles_FiveNodesReceiverTransaction,
        nodeUUID),
    mTrustLinesManager(manager),
    mLogger(logger),
    mInBetweenNodeTopologyMessage(message) {

}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
TransactionResult::SharedConst CyclesFiveNodesResponseTransaction::run() {
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

