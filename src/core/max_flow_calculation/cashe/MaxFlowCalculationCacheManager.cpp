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
    info() << "updateCaches\t" << "mCaches size: " << mCaches.size();
    info() << "updateCaches\t" << "msCaches size: " << msCache.size();

    for (auto &timeAndNodeUUID : msCache) {
        if (utc_now() - timeAndNodeUUID.first > kResetSenderCacheDuration()) {
            NodeUUID* keyUUIDPtr = timeAndNodeUUID.second;
            info() << "updateCaches\t" << *keyUUIDPtr;
            mCaches.erase(*keyUUIDPtr);
            msCache.erase(timeAndNodeUUID.first);
            delete keyUUIDPtr;
        } else {
            break;
        }
    }

    if (mInitiatorCache.first && utc_now() - mInitiatorCache.second > kResetInitiatorCacheDuration()) {
        info() << "updateCaches\t" << "reset Initiator cache";
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

LoggerStream MaxFlowCalculationCacheManager::info() const {

    if (nullptr == mLog)
        throw Exception("logger is not initialised");

    return mLog->info(logHeader());
}

const string MaxFlowCalculationCacheManager::logHeader() const {

    stringstream s;
    s << "[MaxFlowCalculationCacheManager]";

    return s.str();
}
