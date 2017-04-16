#ifndef GEO_NETWORK_CLIENT_BALANCESRESPONCEMESSAGE_H
#define GEO_NETWORK_CLIENT_BALANCESRESPONCEMESSAGE_H
#include "../../Message.hpp"

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"
#include "../../base/transaction/TransactionMessage.h"

class CyclesThreeNodesBalancesResponseMessage: public TransactionMessage {
public:
    typedef shared_ptr<CyclesThreeNodesBalancesResponseMessage> Shared;
public:
    CyclesThreeNodesBalancesResponseMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        uint16_t neighborsUUUIDCount);

    CyclesThreeNodesBalancesResponseMessage(
            const NodeUUID &senderUUID,
            const TransactionUUID &transactionUUID,
            vector<NodeUUID> &neighborsUUUID);

    CyclesThreeNodesBalancesResponseMessage(
            BytesShared buffer);
public:
    pair<BytesShared, size_t> serializeToBytes();

    void addNeighborUUIDAndBalance(NodeUUID neighborUUID);

    const MessageType typeID() const;

    const bool isTransactionMessage() const;

    vector<NodeUUID> NeighborsAndBalances();

protected:
    void deserializeFromBytes(
            BytesShared buffer);

protected:
    vector<NodeUUID> mNeighborsUUUID;
};

#endif //GEO_NETWORK_CLIENT_BALANCESRESPONCEMESSAGE_H
