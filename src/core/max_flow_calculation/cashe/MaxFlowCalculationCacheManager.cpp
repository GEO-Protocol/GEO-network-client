#include "MaxFlowCalculationCacheManager.h"

MaxFlowCalculationCacheManager::MaxFlowCalculationCacheManager(Logger *logger):
    mLog(logger) {
    mInitiatorCache.first = false;
}

void MaxFlowCalculationCacheManager::addCache(const NodeUUID &keyUUID, MaxFlowCalculationCache::Shared cache) {

    NodeUUID* nodeUUIDPtr = new NodeUUID(keyUUID);
    mCaches.insert(make_pair(*nodeUUIDPtr, cache));
    msCache.insert(make_pair(utc_now(), nodeUUIDPtr));
}

MaxFlowCalculationCache::Shared MaxFlowCalculationCacheManager::cacheByNode(
    const NodeUUID &nodeUUID) const {

    auto nodeUUIDAndCache = mCaches.find(nodeUUID);
    if (nodeUUIDAndCache == mCaches.end()) {
        return nullptr;
    }
    return nodeUUIDAndCache->second;
}

void MaxFlowCalculationCacheManager::updateCaches() {
    mLog->logInfo("MaxFlowCalculationCacheManager::updateCaches", "mCaches size: " + to_string(mCaches.size()));
    mLog->logInfo("MaxFlowCalculationCacheManager::updateCaches", "msCaches size: " + to_string(msCache.size()));

    for (auto &timeAndNodeUUID : msCache) {
        if (utc_now() - timeAndNodeUUID.first > kResetCacheDuration()) {
            NodeUUID* keyUUIDPtr = timeAndNodeUUID.second;
            mLog->logInfo("MaxFlowCalculationCacheManager::updateCaches", ((NodeUUID) *keyUUIDPtr).stringUUID());
            mCaches.erase(*keyUUIDPtr);
            msCache.erase(timeAndNodeUUID.first);
            delete keyUUIDPtr;
        } else {
            break;
        }
    }

    if (mInitiatorCache.first && utc_now() - mInitiatorCache.second > kResetInitiatorCacheDuration()) {
        mLog->logInfo("MaxFlowCalculationCacheManager::updateCaches", "reset Initiator cache");
        mInitiatorCache.first = false;
    }
}

void MaxFlowCalculationCacheManager::setInitiatorCache() {
    mInitiatorCache.first = true;
    mInitiatorCache.second = utc_now();
}

bool MaxFlowCalculationCacheManager::isInitiatorCached() {
    return mInitiatorCache.first;
}
