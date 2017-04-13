#ifndef GEO_NETWORK_CLIENT_TRUSTLINESTATESHANDLERTRANSACTION_H
#define GEO_NETWORK_CLIENT_TRUSTLINESTATESHANDLERTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../io/storage/RoutingTableHandler.h"


/*
 * This transaction handles trust line direction modification and removing.
 * It must be launched after every one trust line modification, or removing.
 *
 * Iy will automatically detect if trust line was removed, and, if so -
 * would update routing tables and propagate changes to the rest of neighbor nodes.
 * In case if trust line was changed - it will initialise further routing tables propagation.
 */
class TrustLineStatesHandlerTransaction:
    public BaseTransaction {

public:
    TrustLineStatesHandlerTransaction(
        NodeUUID &contractorUUID,
        TrustLinesManager *trustLines,
        RoutingTableHandler *routingTable2Level,
        Logger *logger);

    TransactionResult::SharedConst run();

protected:
    TransactionResult::SharedConst processTrustLineRemoving();
    TransactionResult::SharedConst processTrustLineUpdating();

protected:
    NodeUUID mContractorNodeUUID;
    TrustLinesManager *mTrustLines;
    RoutingTableHandler *mRoutingTable2Level,
};


#endif //GEO_NETWORK_CLIENT_TRUSTLINESTATESHANDLERTRANSACTION_H
