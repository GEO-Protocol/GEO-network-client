#ifndef GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESBOUNDARYNODES_H
#define GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESBOUNDARYNODES_H

#include "GetTopologyAndBalancesMessageInBetweenNode.h"

class GetTopologyAndBalancesMessageBoundaryNode:
        public GetTopologyAndBalancesMessageInBetweenNode {

    GetTopologyAndBalancesMessageBoundaryNode(
            const TrustLineAmount maxFlow,
            const byte max_depth,
            vector<NodeUUID> &path,
            const BoundaryNodesType boundaryNodes
    );
    GetTopologyAndBalancesMessageBoundaryNode(BytesShared buffer);

public:
    typedef shared_ptr<GetTopologyAndBalancesMessageInBetweenNode> Shared;
    typedef vector<pair<NodeUUID, TrustLineAmount>> BoundaryNodesType;

protected:
//    static const size_t kOffsetToInheritedBytes();
    void deserializeFromBytes(
            BytesShared buffer);

private:
    pair<BytesShared, size_t> serializeToBytes();
    BoundaryNodesType mBoundaryNodes;
};

#endif //GEO_NETWORK_CLIENT_GETTOPOLOGYANDBALANCESBOUNDARYNODES_H
