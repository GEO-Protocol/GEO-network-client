#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHE_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHE_H

#include "../../common/Types.h"
#include "../../common/NodeUUID.h"
#include "../../common/time/TimeUtils.h"

#include <set>

class MaxFlowCalculationCache {

public:
    typedef shared_ptr<MaxFlowCalculationCache> Shared;

    MaxFlowCalculationCache(
        const NodeUUID& nodeUUID,
        const set<NodeUUID> outgoingUUIDs,
        const set<NodeUUID> incomingUUIDs);

    const NodeUUID& nodeUUID() const;

    const DateTime& timeStampCreated() const;

    bool containsIncomingUUID(const NodeUUID& nodeUUID) const;

    bool containsOutgoingUUID(const NodeUUID& nodeUUID) const;

    void addIncomingUUID(const NodeUUID& nodeUUID);

    void addOutgoingUUID(const NodeUUID& nodeUUID);

// todo change on private after testing
public:
    NodeUUID mNodeUUID;
    set<NodeUUID> mIncomingUUIDs;
    set<NodeUUID> mOutgoingUUIDs;
    DateTime mTimeStampCreated;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHE_H
