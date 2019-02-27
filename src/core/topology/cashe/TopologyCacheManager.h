#ifndef GEO_NETWORK_CLIENT_TOPOLOGYCACHEMANAGER_H
#define GEO_NETWORK_CLIENT_TOPOLOGYCACHEMANAGER_H

#include "TopologyCache.h"
#include "../../contractors/addresses/BaseAddress.h"
#include "../../common/time/TimeUtils.h"
#include "../../logger/Logger.h"

#include <map>
#include <unordered_map>

class TopologyCacheManager {

public:
    TopologyCacheManager(
        const SerializedEquivalent equivalent,
        Logger &logger);

    void addCache(
        BaseAddress::Shared keyAddress,
        TopologyCache::Shared cache);

    TopologyCache::Shared cacheByAddress(
        BaseAddress::Shared nodeAddress) const;

    void updateCaches();

    void setInitiatorCache();

    bool isInitiatorCached();

    DateTime closestTimeEvent() const;

    void resetInitiatorCache();

    void removeCache(
        BaseAddress::Shared nodeAddress);

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
    unordered_map<string, TopologyCache::Shared> mCaches;
    map<DateTime, BaseAddress::Shared> msCache;

    pair<bool, DateTime> mInitiatorCache;
    SerializedEquivalent mEquivalent;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_TOPOLOGYCACHEMANAGER_H
