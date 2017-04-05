#ifndef GEO_NETWORK_CLIENT_FOURNODESBALANCESREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_FOURNODESBALANCESREQUESTMESSAGE_H
#include "../../Message.hpp"

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"
#include "../../base/transaction/TransactionMessage.h"

#include "set"

class FourNodesBalancesRequestMessage: public TransactionMessage {
public:
    typedef shared_ptr<FourNodesBalancesRequestMessage> Shared;
public:
    FourNodesBalancesRequestMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
            set<NodeUUID> &neighbors);

    FourNodesBalancesRequestMessage(
            BytesShared buffer);

    const MessageType typeID() const;
    set<NodeUUID> Neighbors();
protected:
    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
            BytesShared buffer);

protected:
    set<NodeUUID> mNeighbors;
};

#endif //GEO_NETWORK_CLIENT_FOURNODESBALANCESREQUESTMESSAGE_H
