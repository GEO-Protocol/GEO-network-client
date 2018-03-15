#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHEMANAGER_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHEMANAGER_H

#include "../../common/NodeUUID.h"
#include "MaxFlowCalculationCache.h"
#include "../../common/time/TimeUtils.h"
#include "../../logger/Logger.h"

#include <map>
#include <unordered_map>
#include <boost/functional/hash.hpp>

class MaxFlowCalculationCacheManager {

public:
    MaxFlowCalculationCacheManager(
        Logger &logger);

    void addCache(
        const NodeUUID &keyUUID,
        MaxFlowCalculationCache::Shared cache);

    MaxFlowCalculationCache::Shared cacheByNode(
        const NodeUUID &nodeUUID) const;

    void updateCaches();

    void setInitiatorCache();

    bool isInitiatorCached();

    DateTime closestTimeEvent() const;

    void resetInitiatorCache();

    void removeCache(
        const NodeUUID &nodeUUID);

private:
    static const byte kResetSenderCacheHours = 0;
    static const byte kResetSenderCacheMinutes = 10;
    static const byte kResetSenderCacheSeconds = 0;

    static Duration& kResetSenderCacheDuration() {
        static auto duration = Duration(
            kResetSenderCacheHours,
            kResetSenderCacheMinutes,
            kResetSenderCacheSeconds);
        return duration;
    }

    static const byte kResetInitiatorCacheHours = 0;
    static const byte kResetInitiatorCacheMinutes = 0;
    static const byte kResetInitiatorCacheSeconds = 30;

    static Duration& kResetInitiatorCacheDuration() {
        static auto duration = Duration(
            kResetInitiatorCacheHours,
            kResetInitiatorCacheMinutes,
            kResetInitiatorCacheSeconds);
        return duration;
    }

private:
    LoggerStream info() const;

    LoggerStream warning() const;

    const string logHeader() const;

private:
    unordered_map<NodeUUID, MaxFlowCalculationCache::Shared, boost::hash<boost::uuids::uuid>> mCaches;
    map<DateTime, NodeUUID*> msCache;
    pair<bool, DateTime> mInitiatorCache;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHEMANAGER_H
