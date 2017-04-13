#include "TrustLineStatesHandlerTransaction.h"

TrustLineStatesHandlerTransaction::TrustLineStatesHandlerTransaction (
    NodeUUID &contractorUUID,
    TrustLinesManager *trustLines,
    RoutingTableHandler *routingTable2Level,
    Logger *logger):

    BaseTransaction(
        BaseTransaction::RoutingTables_TrustLineStatesHandler,
        logger),
    mContractorNodeUUID(contractorUUID),
    mTrustLines(trustLines),
    mRoutingTable2Level(routingTable2Level)
{}

TransactionResult::SharedConst TrustLineStatesHandlerTransaction::run ()
{
    if (not mTrustLines->isNeighbor(mContractorNodeUUID))
        return processTrustLineRemoving();

    return processTrustLineUpdating();
}

TransactionResult::SharedConst TrustLineStatesHandlerTransaction::processTrustLineRemoving()
{
    // Sending notification about removed trust line
    // to each one neighbor node
    for (const auto kNeighborUUIDAndTrustLine : mTrustLines->trustLines())
        if (kNeighborUUIDAndTrustLine.first != mContractorNodeUUID)
            sendMessage()
}
