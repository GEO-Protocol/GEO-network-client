#ifndef GEO_NETWORK_CLIENT_MAXFLOWCACHEMANAGER_H
#define GEO_NETWORK_CLIENT_MAXFLOWCACHEMANAGER_H

#include "../../contractors/addresses/BaseAddress.h"
#include "MaxFlowCache.h"
#include "../../logger/Logger.h"

#include <unordered_map>
#include <map>

class MaxFlowCacheManager {
public:
    MaxFlowCacheManager(
        const SerializedEquivalent equivalent,
        Logger &logger);

    void addCache(
        BaseAddress::Shared keyAddress,
        MaxFlowCache::Shared cache);

    void updateCaches();

    MaxFlowCache::Shared cacheByAddress(
       BaseAddress::Shared nodeAddress) const;

    void updateCache(
        BaseAddress::Shared keyAddress,
        const TrustLineAmount &amount,
        bool isFinal);

    DateTime closestTimeEvent() const;

    void clearCashes();

    // Todo : used only for debug info
    void printCaches();

private:
    LoggerStream info() const;

    LoggerStream warning() const;

    const string logHeader() const;

private:
    static const byte kResetCacheHours = 0;
    static const byte kResetCacheMinutes = 1;
    static const byte kResetCacheSeconds = 0;

    static Duration& kResetCacheDuration() {
        static auto duration = Duration(
            kResetCacheHours,
            kResetCacheMinutes,
            kResetCacheSeconds);
        return duration;
    }

private:
    unordered_map<string, MaxFlowCache::Shared> mCaches;
    map<DateTime, BaseAddress::Shared> mTimeCaches;
    SerializedEquivalent mEquivalent;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCACHEMANAGER_H
