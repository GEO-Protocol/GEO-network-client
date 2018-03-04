#ifndef GEO_NETWORK_CLIENT_ROUTINGTALERESPONSETRANSACTION_H
#define GEO_NETWORK_CLIENT_ROUTINGTALERESPONSETRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../network/messages/routing_table/RoutingTableRequestMessage.h"
#include "../../../network/messages/routing_table/RoutingTableResponseMessage.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"

#include <set>


class RoutingTableResponseTransaction :
    public BaseTransaction {

public:
    RoutingTableResponseTransaction(
        const NodeUUID &nodeUUID,
        RoutingTableRequestMessage::Shared message,
        TrustLinesManager *manager,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

protected:
    RoutingTableRequestMessage::Shared mRequestMessage;
    TrustLinesManager *mTrustLinesManager;
};
#endif //GEO_NETWORK_CLIENT_ROUTINGTALERESPONSETRANSACTION_H
