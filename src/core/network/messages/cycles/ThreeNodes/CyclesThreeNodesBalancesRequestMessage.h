#ifndef GEO_NETWORK_CLIENT_BALANCESREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_BALANCESREQUESTMESSAGE_H

#include "../../Message.hpp"
#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"
#include "../../base/transaction/TransactionMessage.h"
#include "set"
class CyclesThreeNodesBalancesRequestMessage:
        public TransactionMessage {
public:
    typedef shared_ptr<CyclesThreeNodesBalancesRequestMessage> Shared;
public:
    CyclesThreeNodesBalancesRequestMessage(
            const NodeUUID &senderUUID,
            const TransactionUUID &transactionUUID,
            set<NodeUUID> &neighbors);

    CyclesThreeNodesBalancesRequestMessage(
            BytesShared buffer);
public:
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
