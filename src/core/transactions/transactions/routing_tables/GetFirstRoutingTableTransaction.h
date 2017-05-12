#ifndef GEO_NETWORK_CLIENT_GETFIRSTROUTINGTABLETRANSACTION_H
#define GEO_NETWORK_CLIENT_GETFIRSTROUTINGTABLETRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../network/messages/routing_tables/NeighborsRequestMessage.h"
#include "../../../network/messages/routing_tables/NeighborsResponseMessage.h"


class GetFirstRoutingTableTransaction :
    public BaseTransaction {

public:
    using Shared = shared_ptr<GetFirstRoutingTableTransaction>;

public:
    GetFirstRoutingTableTransaction(
        NodeUUID &nodeUUID,
        NeighborsRequestMessage::Shared message,
        TrustLinesManager *manager,
        Logger *logger);

    NeighborsRequestMessage::Shared message() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    NeighborsRequestMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
};


#endif //GEO_NETWORK_CLIENT_GETFIRSTROUTINGTABLETRANSACTION_H
