#ifndef GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESBOUNDARYNODES_H
#define GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESBOUNDARYNODES_H

//TODO:: (D.V.) Of course I understand that all imports already included in parent class,
//TODO:: but in OOP, every class is independent and self-sufficient entity, so please,
//TODO:: include here all dependencies that this class will be using.
#include "InBetweenNodeTopologyMessage.h"

class BoundaryNodeTopologyMessage: public InBetweenNodeTopologyMessage {

public:
    BoundaryNodeTopologyMessage(
            const TrustLineBalance maxFlow,
            const byte max_depth,
            vector<NodeUUID> &path,
            const vector<pair<NodeUUID, TrustLineBalance>> boundaryNodes
    );

    BoundaryNodeTopologyMessage(BytesShared buffer);

public:
    typedef shared_ptr<BoundaryNodeTopologyMessage> Shared;
//    typedef vector<pair<NodeUUID, TrustLineAmount>> BoundaryNodesType;

protected:
//    static const size_t kOffsetToInheritedBytes();
    void deserializeFromBytes(
            BytesShared buffer);

protected:
    pair<BytesShared, size_t> serializeToBytes();
    vector<pair<NodeUUID, TrustLineBalance>> mBoundaryNodes;
};

#endif //GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESBOUNDARYNODES_H
