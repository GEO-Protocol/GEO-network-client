#include "MaxFlowCalculationCacheManager.h"

MaxFlowCalculationCacheManager::MaxFlowCalculationCacheManager() {
    mInitiatorCache.first = false;
}

void MaxFlowCalculationCacheManager::addCache(MaxFlowCalculationCache::Shared cache) {

    mCaches.insert(make_pair(cache->nodeUUID(), cache));
    NodeUUID* nodeUUIDPtr = &(cache->nodeUUID());
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
    cout << "in MaxFlowCalculationCacheManager" << "\n";
    cout << "mCaches size: " << mCaches.size() << "\n";
    cout << "msCaches size: " << msCache.size() << "\n";

    for (auto &timeAndNodeUUID : msCache) {
        if (utc_now() - timeAndNodeUUID.first > kResetCacheDuration()) {
            cout << ((NodeUUID) *timeAndNodeUUID.second).stringUUID() << "\n";
            mCaches.erase(*timeAndNodeUUID.second);
            msCache.erase(timeAndNodeUUID.first);
        } else {
            break;
        }
    }

    if (mInitiatorCache.first && utc_now() - mInitiatorCache.second > kResetInitiatorCacheDuration()) {
        cout << "reset Initiator cache" << "\n";
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
