#include "MaxFlowCalculationNodeCacheManager.h"

MaxFlowCalculationNodeCacheManager::MaxFlowCalculationNodeCacheManager(
    Logger &logger):
    mLog(logger)
{}

void MaxFlowCalculationNodeCacheManager::addCache(const NodeUUID &keyUUID, MaxFlowCalculationNodeCache::Shared cache)
{
    mCaches.insert(
        make_pair(
            keyUUID,
            cache));
}

MaxFlowCalculationNodeCache::Shared MaxFlowCalculationNodeCacheManager::cacheByNode(
    const NodeUUID &nodeUUID) const
{
    auto nodeUUIDAndCache = mCaches.find(nodeUUID);
    if (nodeUUIDAndCache == mCaches.end()) {
        return nullptr;
    }
    return nodeUUIDAndCache->second;
}
