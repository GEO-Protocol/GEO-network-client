#include "GetFirstRoutingTableTransaction.h"


GetFirstRoutingTableTransaction::GetFirstRoutingTableTransaction(
    NodeUUID &nodeUUID,
    NeighborsRequestMessage::Shared message,
    TrustLinesManager *manager,
    Logger &logger):

    BaseTransaction(
        BaseTransaction::TransactionType::RoutingTables_GetFirstRoutingTable,
        nodeUUID,
        logger),

    mMessage(message),
    mTrustLinesManager(manager)
{}

//NeighborsRequestMessage::Shared GetFirstRoutingTableTransaction::message() const
//{
//    return  mMessage;
//}

TransactionResult::SharedConst GetFirstRoutingTableTransaction::run()
{
#ifdef DEBUG_LOG_ROUTING_TABLES_PROCESSING
    debug() << "Neighbors request received from (" << mMessage->senderUUID << ")";
#endif

    const auto kNeighbors = mTrustLinesManager->rt1();
    if (kNeighbors.size() == 1) {

#ifdef DEBUG_LOG_ROUTING_TABLES_PROCESSING
        debug() << "No neighbors (except newly created one) are present. "
                   "No reason to send response. "
                   "Exit.";
#endif

        return resultDone();
    }


    auto neighborsResponseMessage = make_shared<NeighborsResponseMessage>(
        mNodeUUID,
        mMessage->transactionUUID(),
        kNeighbors.size());

    for (const auto &neighbor : kNeighbors) {
        if (neighbor != mMessage->senderUUID) {
            neighborsResponseMessage->appendNeighbor(
                neighbor);
        }
    }

#ifdef DEBUG_LOG_ROUTING_TABLES_PROCESSING
        debug() << "Collected " << neighborsResponseMessage->neighbors().size() << " neighbor node(s).";
#endif

    sendMessage(
        mMessage->senderUUID,
        neighborsResponseMessage);
    return resultDone();
}

const string GetFirstRoutingTableTransaction::logHeader() const
{
    stringstream s;
    s << "[GetFirstRoutingTableTA: " << currentTransactionUUID() << "]";
    return s.str();
}
