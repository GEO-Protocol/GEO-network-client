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
        uint16_t neighborsUUUIDAndBalancesCount);

    CyclesThreeNodesBalancesResponseMessage(
            const NodeUUID &senderUUID,
            const TransactionUUID &transactionUUID,
            vector<pair<NodeUUID, TrustLineBalance>> &neighborsUUUIDAndBalance);

    CyclesThreeNodesBalancesResponseMessage(
            BytesShared buffer);

    void addNeighborUUIDAndBalance(pair<NodeUUID, TrustLineBalance> neighborUUIDAndBalance);
    const MessageType typeID() const;
    const bool isTransactionMessage() const;
    vector<pair<NodeUUID, TrustLineBalance>> NeighborsAndBalances();
protected:
    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
            BytesShared buffer);

protected:
    vector<pair<NodeUUID, TrustLineBalance>> mNeighborsUUUIDAndBalance;
};

#endif //GEO_NETWORK_CLIENT_BALANCESRESPONCEMESSAGE_H
