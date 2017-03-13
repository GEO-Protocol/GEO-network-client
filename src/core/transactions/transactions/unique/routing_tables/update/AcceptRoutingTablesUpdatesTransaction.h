#ifndef GEO_NETWORK_CLIENT_ACCEPTROUTINGTABLESUPDATESTRANSACTION_H
#define GEO_NETWORK_CLIENT_ACCEPTROUTINGTABLESUPDATESTRANSACTION_H

#include "../../../base/BaseTransaction.h"

#include "../../../../../common/Types.h"
#include "../../../../../common/NodeUUID.h"

#include "../../../../../network/messages/Message.hpp"
#include "../../../../../network/messages/outgoing/routing_tables/RoutingTableUpdateOutgoingMessage.h"
#include "../../../../../network/messages/incoming/routing_tables/RoutingTableUpdateIncomingMessage.h"
#include "../../../../../network/messages/response/Response.h"

#include "PropagateRoutingTablesUpdatesTransaction.h"

#include "../../../../../trust_lines/manager/TrustLinesManager.h"

#include <memory>

class AcceptRoutingTablesUpdatesTransaction: public BaseTransaction {
public:
    typedef shared_ptr<AcceptRoutingTablesUpdatesTransaction> Shared;

    AcceptRoutingTablesUpdatesTransaction(
        const NodeUUID &nodeUUID,
        RoutingTableUpdateIncomingMessage::Shared routingTableUpdateMessage,
        TrustLinesManager *trustLinesManager);

    RoutingTableUpdateIncomingMessage::Shared message() const;

    TransactionResult::SharedConst run();

private:
    void updateRoutingTable();

    void tryCreateNextUpdatingTransactionsPool();

    void sendResponseToContractor(
        const uint16_t code);

private:
    const uint16_t kResponseCodeSuccess = 200;

    RoutingTableUpdateIncomingMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
};


#endif //GEO_NETWORK_CLIENT_ACCEPTROUTINGTABLESUPDATESTRANSACTION_H
