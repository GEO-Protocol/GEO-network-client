#ifndef GEO_NETWORK_CLIENT_TRANSACTION_H
#define GEO_NETWORK_CLIENT_TRANSACTION_H


#include "../AbstractRoutingTable.h"
#include "../../../../common/exceptions/RuntimeError.h"


namespace io {
namespace routing_tables {


class AbstractRoutingTable;


class Transaction {
public:
    Transaction(
        const AbstractRoutingTable *routingTable);
    ~Transaction();

    void set(
        const NodeUUID &u1,
        const NodeUUID &u2,
        const TrustLineDirection direction);

    void remove(
        const NodeUUID &u1,
        const NodeUUID &u2);

    void commit();
    void rollback();

protected:
    enum State {
        Normal,
        Broken,
    };

protected:
    State mState;
    AbstractRoutingTable *mRoutingTable;
};


}
}


#endif //GEO_NETWORK_CLIENT_TRANSACTION_H
