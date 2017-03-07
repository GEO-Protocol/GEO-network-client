#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHEMANAGER_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHEMANAGER_H

#include "../../common/NodeUUID.h"
#include "MaxFlowCalculationCache.h"
#include "../../common/time/TimeUtils.h"
#include "../manager/MaxFlowCalculationTrustLineManager.h"

#include <map>
#include <unordered_map>
#include <set>

class MaxFlowCalculationCacheManager {

public:
    MaxFlowCalculationCacheManager();

    void addCache(MaxFlowCalculationCache::Shared cache);

    MaxFlowCalculationCache::Shared cacheByNode(
        const NodeUUID &nodeUUID) const;

    void updateCaches();

    void setInitiatorCache();

    bool isInitiatorCached();

    void testSet();

    void testMap();

    void testMap1();

private:
    static const byte kResetCacheHours = 0;
    static const byte kResetCacheMinutes = 0;
    static const byte kResetCacheSeconds = 10;

    static Duration& kResetCacheDuration() {
        static auto duration = Duration(
            kResetCacheHours,
            kResetCacheMinutes,
            kResetCacheSeconds);
        return duration;
    }

    static const byte kResetInitiatorCacheHours = 0;
    static const byte kResetInitiatorCacheMinutes = 0;
    static const byte kResetInitiatorCacheSeconds = 10;

    static Duration& kResetInitiatorCacheDuration() {
        static auto duration = Duration(
            kResetInitiatorCacheHours,
            kResetInitiatorCacheMinutes,
            kResetInitiatorCacheSeconds);
        return duration;
    }

private:
    // comparing two DateTimes for storing in set
    struct customLess{
        bool operator()(
            const pair<NodeUUID, DateTime> a,
            const pair<NodeUUID, DateTime> b) const {
            return a.second < b.second;
        }
    };

// todo make private after testing
public:
    unordered_map<NodeUUID, MaxFlowCalculationCache::Shared> mCaches;
    map<DateTime, NodeUUID*> msCache;
    pair<bool, DateTime> mInitiatorCache;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHEMANAGER_H
