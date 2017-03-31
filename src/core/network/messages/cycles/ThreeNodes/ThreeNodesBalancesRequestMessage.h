#ifndef GEO_NETWORK_CLIENT_BALANCESREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_BALANCESREQUESTMESSAGE_H

#include "../../Message.hpp"
#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"
#include "../../base/transaction/TransactionMessage.h"
#include "set"
class ThreeNodesBalancesRequestMessage: public TransactionMessage {
public:
    typedef shared_ptr<ThreeNodesBalancesRequestMessage> Shared;
public:
    ThreeNodesBalancesRequestMessage(
            const NodeUUID &senderUUID,
            const TransactionUUID &transactionUUID,
            set<NodeUUID> &neighbors);

    ThreeNodesBalancesRequestMessage(
            BytesShared buffer);

    const MessageType typeID() const;
    vector<NodeUUID> Neighbors();
    pair<BytesShared, size_t> serializeToBytes();

protected:
    void deserializeFromBytes(
            BytesShared buffer);

protected:
    vector<NodeUUID> mNeighbors;
};

#endif //GEO_NETWORK_CLIENT_BALANCESREQUESTMESSAGE_H
