#ifndef GEO_NETWORK_CLIENT_FOURNODESBALANCESREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_FOURNODESBALANCESREQUESTMESSAGE_H
#include "../../Message.hpp"

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"
#include "../../base/transaction/TransactionMessage.h"

class FourNodesBalancesRequestMessage: public TransactionMessage {
public:
    typedef shared_ptr<FourNodesBalancesRequestMessage> Shared;
public:
    FourNodesBalancesRequestMessage(
            const TrustLineBalance& maxFlow,
            vector<NodeUUID> &neighbors);

    FourNodesBalancesRequestMessage(
            BytesShared buffer);

    const MessageType typeID() const;
    vector<NodeUUID> Neighbors();
    TrustLineBalance MaxFlow();
protected:
    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
            BytesShared buffer);

protected:
    vector<NodeUUID> mNeighbors;
    TrustLineBalance mMaxFlow;
};

#endif //GEO_NETWORK_CLIENT_FOURNODESBALANCESREQUESTMESSAGE_H
