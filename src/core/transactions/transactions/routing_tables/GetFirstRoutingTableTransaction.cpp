#include "GetFirstRoutingTableTransaction.h"

GetFirstRoutingTableTransaction::GetFirstRoutingTableTransaction(
    NodeUUID &nodeUUID,
    NeighborsRequestMessage::Shared message,
    TrustLinesManager *manager,
    Logger *logger):

    BaseTransaction(
        BaseTransaction::TransactionType::RoutingTables_GetFirstRoutingTable,
        nodeUUID,
        logger),

    mMessage(message),
    mTrustLinesManager(manager)
{}

NeighborsRequestMessage::Shared GetFirstRoutingTableTransaction::message() const
{
    return  mMessage;
}

TransactionResult::SharedConst GetFirstRoutingTableTransaction::run()
{
    info() << "Get neigbors request receive from " << mMessage->senderUUID;
    auto neighbors = mTrustLinesManager->rt1();
    auto neighborsResponseMessage = make_shared<NeighborsResponseMessage>(
        mNodeUUID,
        mMessage->transactionUUID(),
        neighbors.size());
    for (const auto neighbor : neighbors) {
        if (neighbor != mMessage->senderUUID) {
            neighborsResponseMessage->appendNeighbor(
                neighbor);
        }
    }
    info() << "sending neighors size: " << neighborsResponseMessage->neighbors().size();
    if (neighborsResponseMessage->neighbors().size() > 0) {
        info() << "send message to " << mMessage->senderUUID;
        sendMessage(
            mMessage->senderUUID,
            neighborsResponseMessage);
    }
    return resultDone();
}

const string GetFirstRoutingTableTransaction::logHeader() const
{
    stringstream s;
    s << "[GetFirstRoutingTableTA: " << currentTransactionUUID() << "]";
    return s.str();
}
