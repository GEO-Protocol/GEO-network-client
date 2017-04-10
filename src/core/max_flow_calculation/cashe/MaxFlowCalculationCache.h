#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHE_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHE_H

#include "../../common/Types.h"
#include "../../common/NodeUUID.h"
#include "../../common/time/TimeUtils.h"

#include <unordered_map>
#include <vector>

class MaxFlowCalculationCache {

public:
    typedef shared_ptr<MaxFlowCalculationCache> Shared;

    MaxFlowCalculationCache(
        const vector<pair<NodeUUID, ConstSharedTrustLineAmount>> &outgoingFlows,
        const vector<pair<NodeUUID, ConstSharedTrustLineAmount>> &incomingFlows);

    bool containsIncomingFlow(
        const NodeUUID &nodeUUID,
        ConstSharedTrustLineAmount flow);

    bool containsOutgoingFlow(
        const NodeUUID &nodeUUID,
        ConstSharedTrustLineAmount flow);

private:
    unordered_map<NodeUUID, ConstSharedTrustLineAmount> mIncomingFlows;
    unordered_map<NodeUUID, ConstSharedTrustLineAmount> mOutgoingFlows;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHE_H
