#include "MaxFlowCalculationCacheManager.h"

void MaxFlowCalculationCacheManager::addCache(MaxFlowCalculationCache::Shared cache) {

    mCaches.insert(make_pair(cache->nodeUUID(), cache));
    msCache.insert(make_pair(cache->nodeUUID(), cache->mTimeStampCreated));
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
    /*for (auto nodeUUIDAndCache : mCaches) {
        if (utc_now() - nodeUUIDAndCache.second->mTimeStampCreated > Duration(kTimeHours, kTimeMinutes, kTimeSeconds)) {
            mCaches.erase(nodeUUIDAndCache.first);
            cout << ((NodeUUID) nodeUUIDAndCache.first).stringUUID() << "\n";
        }
    }*/
    for (auto &nodeUUIDAndTime : msCache) {
        if (utc_now() - nodeUUIDAndTime.second > Duration(kTimeHours, kTimeMinutes, kTimeSeconds)) {
            cout << ((NodeUUID) nodeUUIDAndTime.first).stringUUID() << "\n";
            mCaches.erase(nodeUUIDAndTime.first);
            msCache.erase(nodeUUIDAndTime);
        }
    }
}

void MaxFlowCalculationCacheManager::testSet() {

    msCache.insert(make_pair(NodeUUID("11111"), utc_now()));
    msCache.insert(make_pair(NodeUUID("22222"), utc_now()));
    msCache.insert(make_pair(NodeUUID("33333"), utc_now()));

}
