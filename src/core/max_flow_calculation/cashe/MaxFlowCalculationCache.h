#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHE_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHE_H

#include "../../common/Types.h"
#include "../../common/NodeUUID.h"
#include "../../common/time/TimeUtils.h"

#include <map>

class MaxFlowCalculationCache {

public:
    typedef shared_ptr<MaxFlowCalculationCache> Shared;

    MaxFlowCalculationCache(
        const NodeUUID& nodeUUID,
        const map<NodeUUID, TrustLineAmount> outgoingUUIDs,
        const map<NodeUUID, TrustLineAmount> incomingUUIDs);

    const NodeUUID& nodeUUID() const;

    bool containsIncomingFlow(
        const NodeUUID &nodeUUID,
        const TrustLineAmount &flow);

    bool containsOutgoingFlow(
        const NodeUUID &nodeUUID,
        const TrustLineAmount &flow);

// todo change on private after testing
public:
    NodeUUID mNodeUUID;
    map<NodeUUID, TrustLineAmount> mIncomingFlows;
    map<NodeUUID, TrustLineAmount> mOutgoingFlows;
    DateTime mTimeStampCreated;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHE_H
