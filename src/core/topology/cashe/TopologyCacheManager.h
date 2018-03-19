#ifndef GEO_NETWORK_CLIENT_TOPOLOGYCACHEMANAGER_H
#define GEO_NETWORK_CLIENT_TOPOLOGYCACHEMANAGER_H

#include "../../common/NodeUUID.h"
#include "TopologyCache.h"
#include "../../common/time/TimeUtils.h"
#include "../../logger/Logger.h"

#include <map>
#include <unordered_map>
#include <boost/functional/hash.hpp>

class TopologyCacheManager {

public:
    TopologyCacheManager(
        const SerializedEquivalent equivalent,
        Logger &logger);

    void addCache(
        const NodeUUID &keyUUID,
        TopologyCache::Shared cache);

    TopologyCache::Shared cacheByNode(
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
    unordered_map<NodeUUID, TopologyCache::Shared, boost::hash<boost::uuids::uuid>> mCaches;
    map<DateTime, NodeUUID*> msCache;
    pair<bool, DateTime> mInitiatorCache;
    SerializedEquivalent mEquivalent;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_TOPOLOGYCACHEMANAGER_H
