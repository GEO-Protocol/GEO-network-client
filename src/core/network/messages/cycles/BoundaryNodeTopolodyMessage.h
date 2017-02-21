#ifndef GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESBOUNDARYNODES_H
#define GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESBOUNDARYNODES_H

#include "InBetweenNodeTopologyMessage.h"

class BoundaryNodeTopolodyMessage:
        public InBetweenNodeTopologyMessage {

public:
    BoundaryNodeTopolodyMessage(
            const TrustLineBalance maxFlow,
            const byte max_depth,
            vector<NodeUUID> &path,
            const vector<pair<NodeUUID, TrustLineBalance>> boundaryNodes
    );
    BoundaryNodeTopolodyMessage(BytesShared buffer);

public:
    typedef shared_ptr<BoundaryNodeTopolodyMessage> Shared;
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
