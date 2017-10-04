#include "CyclesFiveNodesReceiverTransaction.h"

CyclesFiveNodesReceiverTransaction::CyclesFiveNodesReceiverTransaction(
    const NodeUUID &nodeUUID,
    CyclesFiveNodesInBetweenMessage::Shared message,
    TrustLinesManager *manager,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::TransactionType::Cycles_FiveNodesReceiverTransaction,
        nodeUUID,
        logger
    ),
    mTrustLinesManager(manager),
    mInBetweenNodeTopologyMessage(message)
{}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
TransactionResult::SharedConst CyclesFiveNodesReceiverTransaction::run() {
    vector<NodeUUID> path = mInBetweenNodeTopologyMessage->Path();
    uint8_t currentDepth = path.size();
    TrustLineBalance zeroBalance = 0;
    // Direction has mirror sign for initiator node and receiver node.
    // Direction is calculated based on initiator node
    TrustLineBalance maxFlow = (-1) * mTrustLinesManager->balance(path.back());
    auto firstLevelNodes = mTrustLinesManager->getFirstLevelNodesForCycles(maxFlow);
    if (firstLevelNodes.size() == 0){
        info() << "CyclesFiveNodesReceiverTransaction: No suitable firstLevelNodes " << endl;
        return resultDone();
    }
    bool creditorsBranch = true;
    if (maxFlow < zeroBalance)
        creditorsBranch = false;

    if ((creditorsBranch and currentDepth==1)) {
        mInBetweenNodeTopologyMessage->addNodeToPath(mNodeUUID);
        for(const auto &kNodeUUID: firstLevelNodes)
            sendMessage(
                    kNodeUUID,
                    mInBetweenNodeTopologyMessage
            );
        return resultDone();
    }
    if ((creditorsBranch and currentDepth==2) or (not creditorsBranch and currentDepth==1)){
        path.push_back(mNodeUUID);
        sendMessage<CyclesFiveNodesBoundaryMessage>(
            path.front(),
            path,
            firstLevelNodes
        );
        return resultDone();
    }
    else {
        return resultDone();
    }

}
#pragma clang diagnostic pop
const string CyclesFiveNodesReceiverTransaction::logHeader() const
{
    stringstream s;
    s << "[CyclesFiveNodesReceiverTransactionTA: " << currentTransactionUUID() << "] ";
    return s.str();
}
