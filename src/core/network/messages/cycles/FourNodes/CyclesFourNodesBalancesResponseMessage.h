#ifndef GEO_NETWORK_CLIENT_FOURNODESBALANCESRESPONSEMESSAGE_H
#define GEO_NETWORK_CLIENT_FOURNODESBALANCESRESPONSEMESSAGE_H
#include "../../Message.hpp"

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"
#include "../../base/transaction/TransactionMessage.h"

class CyclesFourNodesBalancesResponseMessage: public TransactionMessage {
public:
    typedef shared_ptr<CyclesFourNodesBalancesResponseMessage> Shared;
public:

    CyclesFourNodesBalancesResponseMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        uint16_t neighborsUUUIDAndBalancesCount);

    CyclesFourNodesBalancesResponseMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
            vector<pair<NodeUUID, TrustLineBalance>> &neighborsBalances
    );

    CyclesFourNodesBalancesResponseMessage(
            BytesShared buffer);

    void AddNeighborUUIDAndBalance(pair<NodeUUID, TrustLineBalance> neighborUUIDAndBalance);
    const MessageType typeID() const;
    const bool isTransactionMessage() const;
    vector<pair<NodeUUID, TrustLineBalance>> NeighborsBalances();
protected:
    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
            BytesShared buffer);


protected:
    vector<pair<NodeUUID, TrustLineBalance>> mNeighborsBalances;
};
#endif //GEO_NETWORK_CLIENT_FOURNODESBALANCESRESPONSEMESSAGE_H
