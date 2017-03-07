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

void MaxFlowCalculationCacheManager::testSet() {

    cout << "test set size set: " << msCache.size() << "\n";

    NodeUUID* nodeUUIDPtr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff91");
    msCache.insert(make_pair(DateTime(boost::posix_time::time_from_string("2016-03-02 09.07.00.0000")), nodeUUIDPtr));
    nodeUUIDPtr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff92");
    msCache.insert(make_pair(DateTime(boost::posix_time::time_from_string("2017-03-02 09.07.00.0000")), nodeUUIDPtr));
    nodeUUIDPtr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff93");
    msCache.insert(make_pair(DateTime(boost::posix_time::time_from_string("2015-03-02 09.07.00.0000")), nodeUUIDPtr));

    for (auto const &it : msCache) {
        cout << it.first << " " << it.second << "\n";
    }

    for (auto &nodeUUIDAndTime : msCache) {
        msCache.erase(nodeUUIDAndTime.first);
    }

    cout << "test set size set after all deleting: " << msCache.size() << "\n";

}

void MaxFlowCalculationCacheManager::testMap() {

    map<TrustLineAmount, NodeUUID> tMap;
    NodeUUID* nodeUUIDPtr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff91");
    tMap.insert(make_pair(TrustLineAmount(20), *nodeUUIDPtr));
    nodeUUIDPtr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff92");
    tMap.insert(make_pair(TrustLineAmount(30), *nodeUUIDPtr));
    nodeUUIDPtr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff93");
    tMap.insert(make_pair(TrustLineAmount(10), *nodeUUIDPtr));
    for (auto const &it : tMap) {
        cout << it.first << " " << it.second << "\n";
    }

}

void MaxFlowCalculationCacheManager::testMap1() {

    map<DateTime, NodeUUID> tMap;
    NodeUUID* nodeUUIDPtr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff91");
    tMap.insert(make_pair(DateTime(boost::posix_time::time_from_string("2016-03-02 09.07.00.0000")), *nodeUUIDPtr));
    nodeUUIDPtr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff92");
    tMap.insert(make_pair(DateTime(boost::posix_time::time_from_string("2017-03-02 09.07.00.0000")), *nodeUUIDPtr));
    nodeUUIDPtr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff93");
    tMap.insert(make_pair(DateTime(boost::posix_time::time_from_string("2015-03-02 09.07.00.0000")), *nodeUUIDPtr));
    for (auto const &it : tMap) {
        cout << it.first << " " << it.second << "\n";
    }

}
