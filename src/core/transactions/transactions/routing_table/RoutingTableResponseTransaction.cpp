#include "RoutingTableResponseTransaction.h"

RoutingTableResponseTransaction::RoutingTableResponseTransaction(
    const NodeUUID &nodeUUID,
    RoutingTableRequestMessage::Shared message,
    TrustLinesManager *manager,
    Logger &logger):
    BaseTransaction(
        BaseTransaction::TransactionType::RoutingTableResponseTransactionType,
        nodeUUID,
        message->equivalent(),
        logger),
    mRequestMessage(message),
    mTrustLinesManager(manager)
{}

TransactionResult::SharedConst RoutingTableResponseTransaction::run()
{
    if(!mTrustLinesManager->isNeighbor(mRequestMessage->senderUUID)){
        warning() << mRequestMessage->senderUUID << " is not a neighbor. Finish transaction";
        return resultDone();
    }
    auto neighbors = mTrustLinesManager->firstLevelNeighbors();
    set<NodeUUID> result;
    for(auto &node:neighbors){
        if(node == mRequestMessage->senderUUID)
            continue;
        result.insert(node);
    }
    sendMessage<RoutingTableResponseMessage>(
        mRequestMessage->senderUUID,
        mEquivalent,
        mNodeUUID,
        result);
    return resultDone();
}

const string RoutingTableResponseTransaction::logHeader() const
{
    stringstream s;
    s << "[RoutingTableResponseTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}
