#ifndef GEO_NETWORK_CLIENT_FOURNODESBALANCESREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_FOURNODESBALANCESREQUESTMESSAGE_H

#include "../../base/transaction/TransactionMessage.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"

#include <set>


class CyclesFourNodesBalancesRequestMessage:
    public TransactionMessage {

public:
    typedef shared_ptr<CyclesFourNodesBalancesRequestMessage> Shared;

public:
    CyclesFourNodesBalancesRequestMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
            set<NodeUUID> &neighbors);

    CyclesFourNodesBalancesRequestMessage(
            BytesShared buffer);

    pair<BytesShared, size_t> serializeToBytes();

    const MessageType typeID() const;

    set<NodeUUID> Neighbors();

protected:
    set<NodeUUID> mNeighbors;
};

#endif //GEO_NETWORK_CLIENT_FOURNODESBALANCESREQUESTMESSAGE_H
