#ifndef GEO_NETWORK_CLIENT_FOURNODESBALANCESRESPONSEMESSAGE_H
#define GEO_NETWORK_CLIENT_FOURNODESBALANCESRESPONSEMESSAGE_H

#include "../../base/transaction/TransactionMessage.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"


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
        vector<NodeUUID> &neighborsUUID);

    CyclesFourNodesBalancesResponseMessage(
        BytesShared buffer);

    pair<BytesShared, size_t> serializeToBytes();

    void AddNeighborUUID(NodeUUID neighborUUID);

    const MessageType typeID() const;

    const bool isTransactionMessage() const
        noexcept;

    vector<NodeUUID> NeighborsUUID();

protected:
    vector<NodeUUID> mNeighborsUUID;
};
#endif //GEO_NETWORK_CLIENT_FOURNODESBALANCESRESPONSEMESSAGE_H
