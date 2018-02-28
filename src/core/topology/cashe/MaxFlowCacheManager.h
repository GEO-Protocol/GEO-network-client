#ifndef GEO_NETWORK_CLIENT_MAXFLOWCACHEMANAGER_H
#define GEO_NETWORK_CLIENT_MAXFLOWCACHEMANAGER_H

#include "../../common/NodeUUID.h"
#include "MaxFlowCache.h"
#include "../../logger/Logger.h"

#include <unordered_map>
#include <boost/functional/hash.hpp>
#include <map>

class MaxFlowCacheManager {
public:
    MaxFlowCacheManager(
        const SerializedEquivalent equivalent,
        Logger &logger);

    void addCache(
        const NodeUUID &keyUUID,
        MaxFlowCache::Shared cache);

    void updateCaches();

    MaxFlowCache::Shared cacheByNode(
        const NodeUUID &nodeUUID) const;

    void updateCache(
        const NodeUUID &keyUUID,
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
    unordered_map<NodeUUID, MaxFlowCache::Shared, boost::hash<boost::uuids::uuid>> mCaches;
    map<DateTime, NodeUUID*> mTimeCaches;
    SerializedEquivalent mEquivalent;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCACHEMANAGER_H
