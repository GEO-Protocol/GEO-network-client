#ifndef GEO_NETWORK_CLIENT_FOURNODESBALANCESREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_FOURNODESBALANCESREQUESTMESSAGE_H
#include "../../Message.hpp"

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"
#include "../../base/transaction/TransactionMessage.h"

#include "set"

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

public:
    pair<BytesShared, size_t> serializeToBytes();

    const MessageType typeID() const;

    set<NodeUUID> Neighbors();

protected:
    void deserializeFromBytes(
            BytesShared buffer);

protected:
    set<NodeUUID> mNeighbors;
};

#endif //GEO_NETWORK_CLIENT_FOURNODESBALANCESREQUESTMESSAGE_H
