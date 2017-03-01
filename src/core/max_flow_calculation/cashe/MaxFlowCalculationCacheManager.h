#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHEMANAGER_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHEMANAGER_H

#include "../../common/NodeUUID.h"
#include "MaxFlowCalculationCache.h"

class MaxFlowCalculationCacheManager {

public:
    void addCache(MaxFlowCalculationCache::Shared cache);

    MaxFlowCalculationCache::Shared cacheByNode(
        const NodeUUID &nodeUUID) const;

    void updateCaches();

private:
    static const byte kTimeHours = 0;
    static const byte kTimeMinutes = 0;
    static const byte kTimeSeconds = 2;
    //static const Duration duration(kTimeHours, kTimeMinutes, kTimeSeconds);

private:
    // comparing two DateTimes for storing in set
    struct {
        bool operator()(
            pair<NodeUUID, DateTime> a,
            pair<NodeUUID, DateTime> b) {
            return a.second < b.second;
        }
    } customLess;

    void testSet();

// todo make private after testing
public:
    map<NodeUUID, MaxFlowCalculationCache::Shared> mCaches;
    set<pair<NodeUUID, DateTime>, customLess> msCache;

};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHEMANAGER_H
