#include "CyclesSixNodesReceiverTransaction.h"

CyclesSixNodesReceiverTransaction::CyclesSixNodesReceiverTransaction(
    const NodeUUID &nodeUUID,
    CyclesSixNodesInBetweenMessage::Shared message,
    TrustLinesManager *manager,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::TransactionType::Cycles_SixNodesReceiverTransaction,
        nodeUUID,
        message->equivalent(),
        logger),
    mTrustLinesManager(manager),
    mInBetweenNodeTopologyMessage(message)
{}

TransactionResult::SharedConst CyclesSixNodesReceiverTransaction::run()
{
    vector<NodeUUID> path = mInBetweenNodeTopologyMessage->Path();
    if (!mTrustLinesManager->trustLineIsActive(path.back())) {
        warning() << "TL with previous node " << path.back() << " is not active";
        return resultDone();
    }
    // Direction has mirror sign for initiator node and receiver node.
    // Direction is calculated based on initiator node
    TrustLineBalance maxFlow = (-1) *  mTrustLinesManager->balance(path.back());

    //  If balance to previous node equal zero finish transaction
    if (maxFlow == TrustLine::kZeroBalance())
        return resultDone();
    auto kFirstLevelNodes = mTrustLinesManager->getFirstLevelNodesForCycles(maxFlow);
    //  Update message path and send to next level nodes
    const auto kCurrentDepth = (SerializedPathLengthSize)path.size();
#ifdef DDEBUG_LOG_CYCLES_BUILDING_POCESSING
    info() << "current depth: " << to_string(kCurrentDepth);
#endif
    if ((kCurrentDepth==1)) {
        mInBetweenNodeTopologyMessage->addNodeToPath(mNodeUUID);
        for(const auto &kNodeUUIDAndBalance: kFirstLevelNodes) {
            sendMessage(
                kNodeUUIDAndBalance,
                mInBetweenNodeTopologyMessage);
        }
    }
    else if (kCurrentDepth==2){
        path.push_back(mNodeUUID);
        sendMessage<CyclesSixNodesBoundaryMessage>(
            path.front(),
            mEquivalent,
            path,
            kFirstLevelNodes);
    }
    else {
        warning() << "Wrong path size " << to_string(kCurrentDepth);
    }
    return resultDone();
}

const string CyclesSixNodesReceiverTransaction::logHeader() const
{
    stringstream s;
    s << "[CyclesSixNodesReceiverTransactionTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}