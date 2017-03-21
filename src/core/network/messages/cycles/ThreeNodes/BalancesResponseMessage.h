#ifndef GEO_NETWORK_CLIENT_BALANCESRESPONCEMESSAGE_H
#define GEO_NETWORK_CLIENT_BALANCESRESPONCEMESSAGE_H
#include "../../Message.hpp"

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"
#include "../../base/transaction/TransactionMessage.h"

class BalancesResponseMessage: public TransactionMessage {
public:
    typedef shared_ptr<BalancesResponseMessage> Shared;
public:
    BalancesResponseMessage();
    BalancesResponseMessage(
            const TrustLineBalance& maxFlow,
            vector<pair<NodeUUID, TrustLineBalance>> &neighbors);

    BalancesResponseMessage(
            BytesShared buffer);

    const MessageType typeID() const;
    vector<pair<NodeUUID, TrustLineBalance>> NeighborsAndBalances();
protected:
    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
            BytesShared buffer);

    static const size_t kOffsetToInheritedBytes();

protected:
    vector<pair<NodeUUID, TrustLineBalance>> mNeighborsBalances;
    TrustLineBalance mMaxFlow;
};

#endif //GEO_NETWORK_CLIENT_BALANCESRESPONCEMESSAGE_H
