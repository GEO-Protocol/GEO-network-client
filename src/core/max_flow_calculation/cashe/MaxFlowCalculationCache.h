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
        const NodeUUID& nodeUUID,
        const vector<pair<NodeUUID, TrustLineAmount>> outgoingFlows,
        const vector<pair<NodeUUID, TrustLineAmount>> incomingFlows);

    NodeUUID& nodeUUID();

    bool containsIncomingFlow(
        const NodeUUID &nodeUUID,
        const TrustLineAmount &flow);

    bool containsOutgoingFlow(
        const NodeUUID &nodeUUID,
        const TrustLineAmount &flow);

// todo change on private after testing
public:
    NodeUUID mNodeUUID;
    unordered_map<NodeUUID, TrustLineAmount> mIncomingFlows;
    unordered_map<NodeUUID, TrustLineAmount> mOutgoingFlows;
    DateTime mTimeStampCreated;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHE_H
