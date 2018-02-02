#include "MaxFlowCalculationNodeCacheManager.h"

MaxFlowCalculationNodeCacheManager::MaxFlowCalculationNodeCacheManager(
    Logger &logger):
    mLog(logger)
{}

void MaxFlowCalculationNodeCacheManager::addCache(
    const NodeUUID &keyUUID,
    MaxFlowCalculationNodeCache::Shared cache)
{
    mCaches.insert(
        make_pair(
            keyUUID,
            cache));
    NodeUUID* nodeUUIDPtr = new NodeUUID(keyUUID);
    mTimeCaches.insert(
        make_pair(
            utc_now(),
            nodeUUIDPtr));
}

void MaxFlowCalculationNodeCacheManager::updateCaches()
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

MaxFlowCalculationNodeCache::Shared MaxFlowCalculationNodeCacheManager::cacheByNode(
    const NodeUUID &nodeUUID) const
{
    auto nodeUUIDAndCache = mCaches.find(nodeUUID);
    if (nodeUUIDAndCache == mCaches.end()) {
        return nullptr;
    }
    return nodeUUIDAndCache->second;
}

void MaxFlowCalculationNodeCacheManager::updateCache(
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

DateTime MaxFlowCalculationNodeCacheManager::closestTimeEvent() const
{
    DateTime result = utc_now() + kResetCacheDuration();
    // if there are caches then take cache removing closest time as result closest time event
    // else take life time of cache + now as result closest time event
    if (mTimeCaches.size() > 0) {
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

void MaxFlowCalculationNodeCacheManager::printCaches()
{
    info() << "printCaches at time " << utc_now();
    for (const auto &nodeCache : mCaches) {
        info() << "Node: " << nodeCache.first;
        info() << "\t" << nodeCache.second->currentFlow() << " "
               << nodeCache.second->isFlowFinal() << " " << nodeCache.second->lastModified();
    }
}

LoggerStream MaxFlowCalculationNodeCacheManager::info() const
{
    return mLog.info(logHeader());
}

LoggerStream MaxFlowCalculationNodeCacheManager::warning() const
{
    return mLog.warning(logHeader());
}

const string MaxFlowCalculationNodeCacheManager::logHeader() const
{
    stringstream s;
    s << "[MaxFlowCalculationNodeCacheManager]";
    return s.str();
}
