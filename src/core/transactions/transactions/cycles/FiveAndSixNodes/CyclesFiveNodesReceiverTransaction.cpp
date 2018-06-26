#include "CyclesFiveNodesReceiverTransaction.h"

CyclesFiveNodesReceiverTransaction::CyclesFiveNodesReceiverTransaction(
    const NodeUUID &nodeUUID,
    CyclesFiveNodesInBetweenMessage::Shared message,
    TrustLinesManager *manager,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::TransactionType::Cycles_FiveNodesReceiverTransaction,
        nodeUUID,
        message->equivalent(),
        logger),
    mTrustLinesManager(manager),
    mInBetweenNodeTopologyMessage(message)
{}

TransactionResult::SharedConst CyclesFiveNodesReceiverTransaction::run()
{
    vector<NodeUUID> path = mInBetweenNodeTopologyMessage->Path();
    if (!mTrustLinesManager->trustLineIsActive(path.back())) {
        warning() << "TL with previous node " << path.back() << " is not active";
        return resultDone();
    }
    // Direction has mirror sign for initiator node and receiver node.
    // Direction is calculated based on initiator node
    TrustLineBalance maxFlow = (-1) * mTrustLinesManager->balance(path.back());
    auto firstLevelNodes = mTrustLinesManager->getFirstLevelNodesForCycles(maxFlow);
    if (firstLevelNodes.empty()){
        info() << "CyclesFiveNodesReceiverTransaction: No suitable firstLevelNodes " << endl;
        return resultDone();
    }
    bool creditorsBranch = true;
    if (maxFlow < TrustLine::kZeroBalance())
        creditorsBranch = false;

    auto currentDepth = (SerializedPathLengthSize)path.size();
    if ((creditorsBranch and currentDepth==1)) {
        mInBetweenNodeTopologyMessage->addNodeToPath(mNodeUUID);
        for(const auto &kNodeUUID: firstLevelNodes)
            sendMessage(
                kNodeUUID,
                mInBetweenNodeTopologyMessage);
        return resultDone();
    }
    if ((creditorsBranch and currentDepth==2) or (not creditorsBranch and currentDepth==1)){
        path.push_back(mNodeUUID);
        sendMessage<CyclesFiveNodesBoundaryMessage>(
            path.front(),
            mEquivalent,
            path,
            firstLevelNodes);
        return resultDone();
    }
    else {
        return resultDone();
    }

}

const string CyclesFiveNodesReceiverTransaction::logHeader() const
{
    stringstream s;
    s << "[CyclesFiveNodesReceiverTransactionTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}
