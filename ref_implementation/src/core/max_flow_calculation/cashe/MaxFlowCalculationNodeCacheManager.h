/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONNODECACHEMANAGER_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONNODECACHEMANAGER_H

#include "../../common/NodeUUID.h"
#include "MaxFlowCalculationNodeCache.h"
#include "../../logger/Logger.h"

#include <unordered_map>
#include <boost/functional/hash.hpp>
#include <map>

class MaxFlowCalculationNodeCacheManager {
public:
    MaxFlowCalculationNodeCacheManager(
        Logger &logger);

    void addCache(
        const NodeUUID &keyUUID,
        MaxFlowCalculationNodeCache::Shared cache);

    void updateCaches();

    MaxFlowCalculationNodeCache::Shared cacheByNode(
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
    unordered_map<NodeUUID, MaxFlowCalculationNodeCache::Shared, boost::hash<boost::uuids::uuid>> mCaches;
    map<DateTime, NodeUUID*> mTimeCaches;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONNODECACHEMANAGER_H
