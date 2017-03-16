#ifndef GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESBOUNDARYNODES_H
#define GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESBOUNDARYNODES_H
#include "InBetweenNodeTopologyMessage.h"

class BoundaryNodeTopologyMessage: public InBetweenNodeTopologyMessage {
public:
    typedef shared_ptr<BoundaryNodeTopologyMessage> Shared;

public:
    BoundaryNodeTopologyMessage(
            const CycleType cycleType,
            const TrustLineBalance &maxFlow,
            const byte &max_depth,
            vector<NodeUUID> &path,
            const vector<pair<NodeUUID, TrustLineBalance>> &boundaryNodes);

    BoundaryNodeTopologyMessage(
        BytesShared buffer);

    const MessageType typeID() const;
    const bool isCyclesDiscoveringResponseMessage() const;
    const vector<pair<NodeUUID, TrustLineBalance>> BoundaryNodes() const;


protected:
    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
            BytesShared buffer);


private:
    vector<pair<NodeUUID, TrustLineBalance>> mBoundaryNodes;
};

#endif //GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESBOUNDARYNODES_H
