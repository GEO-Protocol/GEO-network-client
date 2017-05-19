#include "MaxFlowCalculationCacheManager.h"

MaxFlowCalculationCacheManager::MaxFlowCalculationCacheManager(Logger *logger):
    mLog(logger)
{
    mInitiatorCache.first = false;
}

void MaxFlowCalculationCacheManager::addCache(const NodeUUID &keyUUID, MaxFlowCalculationCache::Shared cache)
{
    NodeUUID* nodeUUIDPtr = new NodeUUID(keyUUID);
    mCaches.insert(make_pair(*nodeUUIDPtr, cache));
    msCache.insert(make_pair(utc_now(), nodeUUIDPtr));
}

MaxFlowCalculationCache::Shared MaxFlowCalculationCacheManager::cacheByNode(
    const NodeUUID &nodeUUID) const
{
    auto nodeUUIDAndCache = mCaches.find(nodeUUID);
    if (nodeUUIDAndCache == mCaches.end()) {
        return nullptr;
    }
    return nodeUUIDAndCache->second;
}

void MaxFlowCalculationCacheManager::updateCaches()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "updateCaches\t" << "mCaches size: " << mCaches.size();
    info() << "updateCaches\t" << "msCaches size: " << msCache.size();
#endif
    for (auto &timeAndNodeUUID : msCache) {
        if (utc_now() - timeAndNodeUUID.first > kResetSenderCacheDuration()) {
            NodeUUID* keyUUIDPtr = timeAndNodeUUID.second;
#ifdef  DEBUG_LOG_MAX_FLOW_CALCULATION
            info() << "updateCaches\t" << *keyUUIDPtr;
#endif
            mCaches.erase(*keyUUIDPtr);
            msCache.erase(timeAndNodeUUID.first);
            delete keyUUIDPtr;
        } else {
            break;
        }
    }
    if (mInitiatorCache.first && utc_now() - mInitiatorCache.second > kResetInitiatorCacheDuration()) {
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
        info() << "updateCaches\t" << "reset Initiator cache";
#endif
        mInitiatorCache.first = false;
    }
}

void MaxFlowCalculationCacheManager::setInitiatorCache()
{
    mInitiatorCache.first = true;
    mInitiatorCache.second = utc_now();
}

void MaxFlowCalculationCacheManager::resetInititorCache()
{
    mInitiatorCache.first = false;
}

bool MaxFlowCalculationCacheManager::isInitiatorCached()
{
    return mInitiatorCache.first;
}

DateTime MaxFlowCalculationCacheManager::closestTimeEvent() const
{
    // if initiator cache is active then take initiator cache removing time as result closest time event
    // else take life time of initiator cache + now as result closest time event
    DateTime result = utc_now() + kResetInitiatorCacheDuration();
    if (mInitiatorCache.first && mInitiatorCache.second + kResetInitiatorCacheDuration() < result) {
        result = mInitiatorCache.second + kResetInitiatorCacheDuration();
    }
    // if there are sender caches then take sender cache removing closest time as result closest time event
    // else take life time of sender cache + now as result closest time event
    // take as result minimal from initiator cache and sender cache closest time
    if (msCache.size() > 0) {
        auto timeAndNodeUUID = msCache.cbegin();
        if (timeAndNodeUUID->first + kResetSenderCacheDuration() < result) {
            result = timeAndNodeUUID->first + kResetSenderCacheDuration();
        }
    } else {
        if (utc_now() + kResetSenderCacheDuration() < result) {
            result = utc_now() + kResetSenderCacheDuration();
        }
    }
    return result;
}

LoggerStream MaxFlowCalculationCacheManager::info() const
{
    if (nullptr == mLog)
        throw Exception("logger is not initialised");
    return mLog->info(logHeader());
}

const string MaxFlowCalculationCacheManager::logHeader() const
{
    stringstream s;
    s << "[MaxFlowCalculationCacheManager]";
    return s.str();
}
