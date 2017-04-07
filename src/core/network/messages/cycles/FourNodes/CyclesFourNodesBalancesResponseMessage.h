#ifndef GEO_NETWORK_CLIENT_FOURNODESBALANCESRESPONSEMESSAGE_H
#define GEO_NETWORK_CLIENT_FOURNODESBALANCESRESPONSEMESSAGE_H
#include "../../Message.hpp"

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"
#include "../../base/transaction/TransactionMessage.h"

class CyclesFourNodesBalancesResponseMessage:
        public TransactionMessage {
public:
    typedef shared_ptr<CyclesFourNodesBalancesResponseMessage> Shared;
public:

    CyclesFourNodesBalancesResponseMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        uint16_t neighborsUUUIDCount);

    CyclesFourNodesBalancesResponseMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        vector<NodeUUID> &neighborsUUID
    );

    CyclesFourNodesBalancesResponseMessage(
            BytesShared buffer);

public:
    pair<BytesShared, size_t> serializeToBytes();

    void AddNeighborUUID(NodeUUID neighborUUID);

    const MessageType typeID() const;

    const bool isTransactionMessage() const;

    vector<NodeUUID> NeighborsUUID();

protected:

    void deserializeFromBytes(
            BytesShared buffer);

protected:
    vector<NodeUUID> mNeighborsUUID;
};
#endif //GEO_NETWORK_CLIENT_FOURNODESBALANCESRESPONSEMESSAGE_H
