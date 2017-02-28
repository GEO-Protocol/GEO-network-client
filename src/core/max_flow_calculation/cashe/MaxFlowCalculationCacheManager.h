#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHEMANAGER_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHEMANAGER_H

#include "../../common/NodeUUID.h"
#include "MaxFlowCalculationCache.h"

class MaxFlowCalculationCacheManager {

public:
    void addCache(MaxFlowCalculationCache::Shared cache);

    bool containsNodeUUID(const NodeUUID& nodeUUID) const;

    bool containsIncomingFlow(
        const NodeUUID& fstNodeUUID,
        const NodeUUID& sndNodeUUID) const;

    bool containsOutgoingFlow(
        const NodeUUID& fstNodeUUID,
        const NodeUUID& sndNodeUUID) const;

// todo make private after testing
public:
    map<NodeUUID, MaxFlowCalculationCache::Shared> mCaches;

};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHEMANAGER_H
