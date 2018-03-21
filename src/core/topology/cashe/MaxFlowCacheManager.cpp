#include "MaxFlowCacheManager.h"

MaxFlowCacheManager::MaxFlowCacheManager(
    const SerializedEquivalent equivalent,
    Logger &logger):

    mEquivalent(equivalent),
    mLog(logger)
{}

void MaxFlowCacheManager::addCache(
    const NodeUUID &keyUUID,
    MaxFlowCache::Shared cache)
{
    auto nodeUUIDPtr = new NodeUUID(keyUUID);
    mCaches.insert(
        make_pair(
            *nodeUUIDPtr,
            cache));
    mTimeCaches.insert(
        make_pair(
            utc_now(),
            nodeUUIDPtr));
}

void MaxFlowCacheManager::updateCaches()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "updateCaches\t" << "mCaches size: " << mCaches.size();
    info() << "updateCaches\t" << "msCaches size: " << mTimeCaches.size();
#endif
    for (auto &timeAndNodeUUID : mTimeCaches) {
        if (utc_now() - timeAndNodeUUID.first > kResetCacheDuration()) {
            NodeUUID* keyUUIDPtr = timeAndNodeUUID.second;
#ifdef  DEBUG_LOG_MAX_FLOW_CALCULATION
            info() << "updateCaches delete cache\t" << *keyUUIDPtr;
#endif
            mCaches.erase(*keyUUIDPtr);
            mTimeCaches.erase(timeAndNodeUUID.first);
            delete keyUUIDPtr;
        } else {
            break;
        }
    }
}

MaxFlowCache::Shared MaxFlowCacheManager::cacheByNode(
    const NodeUUID &nodeUUID) const
{
    auto nodeUUIDAndCache = mCaches.find(nodeUUID);
    if (nodeUUIDAndCache == mCaches.end()) {
        return nullptr;
    }
    return nodeUUIDAndCache->second;
}

void MaxFlowCacheManager::updateCache(
    const NodeUUID &keyUUID,
    const TrustLineAmount &amount,
    bool isFinal)
{
    auto nodeUUIDAndCache = mCaches.find(keyUUID);
    if (nodeUUIDAndCache == mCaches.end()) {
        warning() << "Try update cache which is absent in map";
        return;
    }
    nodeUUIDAndCache->second->updateCurrentFlow(
        amount,
        isFinal);
}

DateTime MaxFlowCacheManager::closestTimeEvent() const
{
    DateTime result = utc_now() + kResetCacheDuration();
    // if there are caches then take cache removing closest time as result closest time event
    // else take life time of cache + now as result closest time event
    if (!mTimeCaches.empty()) {
        auto timeAndNodeUUID = mTimeCaches.cbegin();
        if (timeAndNodeUUID->first + kResetCacheDuration() < result) {
            result = timeAndNodeUUID->first + kResetCacheDuration();
        }
    } else {
        if (utc_now() + kResetCacheDuration() < result) {
            result = utc_now() + kResetCacheDuration();
        }
    }
    return result;
}

void MaxFlowCacheManager::clearCashes()
{
    for (auto cacheElement : mTimeCaches) {
        delete cacheElement.second;
    }
    mTimeCaches.clear();
    mCaches.clear();
}

void MaxFlowCacheManager::printCaches()
{
    info() << "printCaches at time " << utc_now();
    for (const auto &nodeCache : mCaches) {
        info() << "Node: " << nodeCache.first;
        info() << "\t" << nodeCache.second->currentFlow() << " "
               << nodeCache.second->isFlowFinal() << " " << nodeCache.second->lastModified();
    }
}

LoggerStream MaxFlowCacheManager::info() const
{
    return mLog.info(logHeader());
}

LoggerStream MaxFlowCacheManager::warning() const
{
    return mLog.warning(logHeader());
}

const string MaxFlowCacheManager::logHeader() const
{
    stringstream s;
    s << "[MaxFlowCacheManager: " << mEquivalent << "] ";
    return s.str();
}
