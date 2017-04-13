#include "CyclesSixNodesReceiverTransaction.h"
#include "../../../../network/messages/cycles/SixAndFiveNodes/CyclesFiveNodesBoundaryMessage.hpp"

CyclesSixNodesReceiverTransaction::CyclesSixNodesReceiverTransaction(
    const NodeUUID &nodeUUID,
    CyclesSixNodesInBetweenMessage::Shared message,
    TrustLinesManager *manager,
    Logger *logger) :
    BaseTransaction(
        BaseTransaction::TransactionType::Cycles_SixNodesReceiverTransaction,
        nodeUUID),
    mTrustLinesManager(manager),
    mLogger(logger),
    mInBetweenNodeTopologyMessage(message) {
}

TransactionResult::SharedConst CyclesSixNodesReceiverTransaction::run() {
    vector<NodeUUID> path = mInBetweenNodeTopologyMessage->Path();
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
    const uint8_t kCurrentDepth = path.size();
#pragma clang diagnostic pop
    const TrustLineBalance kZeroBalance = 0;
    // Direction has mirror sign for initiator node and receiver node.
    // Direction is calculated based on initiator node
    TrustLineBalance maxFlow = (-1) *  mTrustLinesManager->balance(path.back());

//  If balance to previous node equal zero finish transaction
    if (maxFlow == kZeroBalance)
        return resultExit();
    auto kFirstLevelNodes = mTrustLinesManager->getFirstLevelNodesForCycles(maxFlow);
//  Update message path and send to next level nodes
    if ((kCurrentDepth==1)) {
        mInBetweenNodeTopologyMessage->addNodeToPath(mNodeUUID);
        for(const auto &kNodeUUIDAndBalance: kFirstLevelNodes)
            sendMessage(
                kNodeUUIDAndBalance,
                mInBetweenNodeTopologyMessage
            );
    }
    if (kCurrentDepth==2){
        path.push_back(mNodeUUID);
        sendMessage<CyclesFiveNodesBoundaryMessage>(
            path.front(),
            path,
            kFirstLevelNodes
        );
    }
    else {
        mLogger->error("CyclesSixNodesReceiverTransaction:"
        "Wrong path size");
    }
    return resultExit();
}