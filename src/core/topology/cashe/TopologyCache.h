#ifndef GEO_NETWORK_CLIENT_TOPOLOGYCACHE_H
#define GEO_NETWORK_CLIENT_TOPOLOGYCACHE_H

#include "../../common/Types.h"
#include "../../common/NodeUUID.h"
#include "../../trust_lines/TrustLine.h"
#include "../../common/time/TimeUtils.h"

#include <unordered_map>
#include <vector>
#include <boost/functional/hash.hpp>

class TopologyCache {

public:
    typedef shared_ptr<TopologyCache> Shared;

    TopologyCache(
        const vector<pair<NodeUUID, ConstSharedTrustLineAmount>> &outgoingFlows,
        const vector<pair<NodeUUID, ConstSharedTrustLineAmount>> &incomingFlows);

    bool containsIncomingFlow(
        const NodeUUID &nodeUUID,
        ConstSharedTrustLineAmount flow);

    bool containsOutgoingFlow(
        const NodeUUID &nodeUUID,
        ConstSharedTrustLineAmount flow);

private:
    unordered_map<NodeUUID, ConstSharedTrustLineAmount, boost::hash<boost::uuids::uuid>> mIncomingFlows;
    unordered_map<NodeUUID, ConstSharedTrustLineAmount, boost::hash<boost::uuids::uuid>> mOutgoingFlows;
};


#endif //GEO_NETWORK_CLIENT_TOPOLOGYCACHE_H
