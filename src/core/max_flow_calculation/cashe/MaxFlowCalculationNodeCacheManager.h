#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONNODECACHEMANAGER_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONNODECACHEMANAGER_H

#include "../../common/NodeUUID.h"
#include "MaxFlowCalculationNodeCache.h"
#include "../../logger/Logger.h"

#include <unordered_map>
#include <boost/functional/hash.hpp>


class MaxFlowCalculationNodeCacheManager {
public:
    MaxFlowCalculationNodeCacheManager(
        Logger &logger);

    void addCache(
        const NodeUUID &keyUUID,
        MaxFlowCalculationNodeCache::Shared cache);

    MaxFlowCalculationNodeCache::Shared cacheByNode(
        const NodeUUID &nodeUUID) const;

private:
    unordered_map<NodeUUID, MaxFlowCalculationNodeCache::Shared, boost::hash<boost::uuids::uuid>> mCaches;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONNODECACHEMANAGER_H
