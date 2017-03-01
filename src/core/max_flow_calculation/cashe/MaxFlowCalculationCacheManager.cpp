#include "MaxFlowCalculationCacheManager.h"

void MaxFlowCalculationCacheManager::addCache(MaxFlowCalculationCache::Shared cache) {

    mCaches.insert(make_pair(cache->nodeUUID(), cache));
}

bool MaxFlowCalculationCacheManager::containsNodeUUID(const NodeUUID &nodeUUID) const {

    if (mCaches.find(nodeUUID) == mCaches.end()) {
        return false;
    } else {
        return true;
    }
}

bool MaxFlowCalculationCacheManager::containsOutgoingFlow(
    const NodeUUID &fstNodeUUID,
    const NodeUUID &sndNodeUUID) const {

    auto const& nodeUUIDAndCache = mCaches.find(fstNodeUUID);
    if (nodeUUIDAndCache == mCaches.end()) {
        return false;
    }
    return nodeUUIDAndCache->second->containsOutgoingUUID(sndNodeUUID);
}

bool MaxFlowCalculationCacheManager::containsIncomingFlow(
    const NodeUUID &fstNodeUUID,
    const NodeUUID &sndNodeUUID) const {

    auto const& nodeUUIDAndCache = mCaches.find(fstNodeUUID);
    if (nodeUUIDAndCache == mCaches.end()) {
        return false;
    }
    return nodeUUIDAndCache->second->containsIncomingUUID(sndNodeUUID);
}

void MaxFlowCalculationCacheManager::updateCaches() {
    for (auto nodeUUIDAndCache : mCaches) {
        // todo if time out duration
        mCaches.erase(nodeUUIDAndCache.first);
    }
}
