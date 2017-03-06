#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLEUPDATETRANSACTIONSFACTORY_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLEUPDATETRANSACTIONSFACTORY_H

#include "../../../base/BaseTransaction.h"

#include "../../../../../common/Types.h"
#include "../../../../../common/NodeUUID.h"

#include "../../../../../network/messages/outgoing/routing_tables/RoutingTableUpdateOutgoingMessage.h"

#include "PropagateRoutingTablesUpdatesTransaction.h"

#include "../../../../../trust_lines/manager/TrustLinesManager.h"

#include <memory>


class RoutingTablesUpdateTransactionsFactory: public BaseTransaction {
public:
    typedef shared_ptr<RoutingTablesUpdateTransactionsFactory> Shared;

public:
    RoutingTablesUpdateTransactionsFactory(
        const NodeUUID &nodeUUID,
        const NodeUUID &contractorUUID,
        const TrustLineDirection direction,
        TrustLinesManager *trustLinesManager);

private:
    TransactionResult::SharedConst run();

    void createRoutingTablesUpdatePoolTransactions();

private:
    NodeUUID mContractorUUID;
    TrustLineDirection mDirection;
    TrustLinesManager *mTrustLinesManager;
};


#endif //GEO_NETWORK_CLIENT_ROUTINGTABLEUPDATETRANSACTIONSFACTORY_H
