#ifndef GEO_NETWORK_CLIENT_BALANCESREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_BALANCESREQUESTMESSAGE_H

#include "../../Message.hpp"

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"
#include "../../base/transaction/TransactionMessage.h"

class BalancesRequestMessage: public TransactionMessage {
public:
    typedef shared_ptr<BalancesRequestMessage> Shared;
public:
    BalancesRequestMessage();
    BalancesRequestMessage(
            const TrustLineBalance& maxFlow,
            vector<NodeUUID> &neighbors);

    BalancesRequestMessage(
            BytesShared buffer);

    const MessageType typeID() const;
    vector<NodeUUID> Neighbors();
protected:
    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
            BytesShared buffer);

    static const size_t kOffsetToInheritedBytes();


protected:
    vector<NodeUUID> mNeighbors;
    TrustLineBalance mMaxFlow;
};

#endif //GEO_NETWORK_CLIENT_BALANCESREQUESTMESSAGE_H
