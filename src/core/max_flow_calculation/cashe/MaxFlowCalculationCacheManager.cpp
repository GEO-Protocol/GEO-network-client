#include "MaxFlowCalculationCacheManager.h"

MaxFlowCalculationCacheManager::MaxFlowCalculationCacheManager() {
    mInitiatorCache.first = false;
}

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
        if (utc_now() - nodeUUIDAndCache.second->mTimeStampCreated > Duration(kResetCacheHours, kResetCacheMinutes, kResetCacheSeconds)) {
            mCaches.erase(nodeUUIDAndCache.first);
            cout << ((NodeUUID) nodeUUIDAndCache.first).stringUUID() << "\n";
        }
    }*/
    for (auto &nodeUUIDAndTime : msCache) {
        if (utc_now() - nodeUUIDAndTime.second > Duration(kResetCacheHours, kResetCacheMinutes, kResetCacheSeconds)) {
            cout << ((NodeUUID) nodeUUIDAndTime.first).stringUUID() << "\n";
            mCaches.erase(nodeUUIDAndTime.first);
            msCache.erase(nodeUUIDAndTime);
        }
    }

    if (mInitiatorCache.first && utc_now() - mInitiatorCache.second > Duration(
        kResetInitiatorCacheHours, kResetInitiatorCacheMinutes, kResetInitiatorCacheSeconds)) {
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

void MaxFlowCalculationCacheManager::testSet() {

    cout << "test set size set: " << msCache.size() << "\n";

    NodeUUID* nodeUUIDPtr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff91");
    msCache.insert(make_pair(*nodeUUIDPtr,
                             DateTime(boost::posix_time::time_from_string("2016-03-02 09.07.00.0000"))));
    nodeUUIDPtr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff92");
    msCache.insert(make_pair(*nodeUUIDPtr,
                             DateTime(boost::posix_time::time_from_string("2017-03-02 09.07.00.0000"))));
    nodeUUIDPtr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff93");
    msCache.insert(make_pair(*nodeUUIDPtr,
                             DateTime(boost::posix_time::time_from_string("2015-03-02 09.07.00.0000"))));

    for (auto const &it : msCache) {
        cout << it.first << " " << it.second << "\n";
    }

}
