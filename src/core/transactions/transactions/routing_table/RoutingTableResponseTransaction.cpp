#include "RoutingTableResponseTransaction.h"

RoutingTableResponseTransaction::RoutingTableResponseTransaction(
    const NodeUUID &nodeUUID,
    RoutingTableRequestMessage::Shared message,
    TrustLinesManager *manager,
    Logger &logger):
    BaseTransaction(
        BaseTransaction::TransactionType::RoutingTableResponceTransactionType,
        nodeUUID,
        logger),
    mRequestMessage(message),
    mTrustLinesManager(manager)
{

}

const string RoutingTableResponseTransaction::logHeader() const
{
    stringstream s;
    s << "[RoutingTableResponseTA: " << currentTransactionUUID() << "] ";
    return s.str();
}

TransactionResult::SharedConst RoutingTableResponseTransaction::run()
{
    if(!mTrustLinesManager->isNeighbor(mRequestMessage->senderUUID)){
        info() << mRequestMessage->senderUUID << " is not a neighbor. Finish transaction;";
    }
    auto neighbors = mTrustLinesManager->rt1();
    set<NodeUUID> result;
    for(auto node:neighbors){
        if(node == mRequestMessage->senderUUID)
            continue;
        result.insert(node);
    }
    sendMessage<RoutingTableResponseMessage>(
        mRequestMessage->senderUUID,
        mNodeUUID,
        result
    );
    return resultDone();
}
