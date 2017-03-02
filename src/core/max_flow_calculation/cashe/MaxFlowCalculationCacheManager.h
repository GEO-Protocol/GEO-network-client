#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHEMANAGER_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHEMANAGER_H

#include "../../common/NodeUUID.h"
#include "MaxFlowCalculationCache.h"
#include "../../common/time/TimeUtils.h"

#include <map>
#include <set>
/*#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>*/

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

private:
    static const byte kResetCacheHours = 0;
    static const byte kResetCacheMinutes = 0;
    static const byte kResetCacheSeconds = 2;
    //static const Duration duration(kResetCacheHours, kResetCacheMinutes, kResetCacheSeconds);

    static const byte kResetInitiatorCacheHours = 0;
    static const byte kResetInitiatorCacheMinutes = 0;
    static const byte kResetInitiatorCacheSeconds = 2;

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
    map<NodeUUID, MaxFlowCalculationCache::Shared> mCaches;
    set<pair<NodeUUID, DateTime>, customLess> msCache;
    pair<bool, DateTime> mInitiatorCache;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHEMANAGER_H
