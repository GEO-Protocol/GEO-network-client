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

void MaxFlowCacheManager::addCacheNew(
    BaseAddress::Shared keyAddress,
    MaxFlowCache::Shared cache)
{
    mCachesNew.insert(
        make_pair(
            keyAddress->fullAddress(),
            cache));
    mTimeCachesNew.insert(
        make_pair(
            utc_now(),
            keyAddress));
}

void MaxFlowCacheManager::updateCaches()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "updateCaches\t" << "mCaches size: " << mCaches.size();
    info() << "updateCaches\t" << "msCaches size: " << mTimeCaches.size();
    info() << "updateCaches\t" << "mCachesNew size: " << mCachesNew.size();
    info() << "updateCaches\t" << "msCachesNew size: " << mTimeCachesNew.size();
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

    for (auto &timeAndNodeAddress : mTimeCachesNew) {
        if (utc_now() - timeAndNodeAddress.first > kResetCacheDuration()) {
            auto keyAddressPtr = timeAndNodeAddress.second;
#ifdef  DEBUG_LOG_MAX_FLOW_CALCULATION
            info() << "updateCaches delete cache\t" << keyAddressPtr->fullAddress();
#endif
            mCachesNew.erase(keyAddressPtr->fullAddress());
            mTimeCachesNew.erase(timeAndNodeAddress.first);
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

MaxFlowCache::Shared MaxFlowCacheManager::cacheByAddress(
    BaseAddress::Shared nodeAddress) const
{
    auto nodeAddressAndCache = mCachesNew.find(nodeAddress->fullAddress());
    if (nodeAddressAndCache == mCachesNew.end()) {
        return nullptr;
    }
    return nodeAddressAndCache->second;
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

void MaxFlowCacheManager::updateCacheNew(
    BaseAddress::Shared keyAddress,
    const TrustLineAmount &amount,
    bool isFinal)
{
    auto nodeAddressAndCache = mCachesNew.find(keyAddress->fullAddress());
    if (nodeAddressAndCache == mCachesNew.end()) {
        warning() << "Try update cache which is absent in map";
        return;
    }
    nodeAddressAndCache->second->updateCurrentFlow(
        amount,
        isFinal);
}

DateTime MaxFlowCacheManager::closestTimeEvent() const
{
    DateTime result = utc_now() + kResetCacheDuration();
    // if there are caches then take cache removing closest time as result closest time event
    // else take life time of cache + now as result closest time event
    if (!mTimeCachesNew.empty()) {
        auto timeAndNodeAddress = mTimeCachesNew.cbegin();
        if (timeAndNodeAddress->first + kResetCacheDuration() < result) {
            result = timeAndNodeAddress->first + kResetCacheDuration();
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

    mTimeCachesNew.clear();
    mCachesNew.clear();
}

void MaxFlowCacheManager::printCaches()
{
    info() << "printCaches at time " << utc_now();
    for (const auto &nodeCache : mCaches) {
        info() << "Node: " << nodeCache.first;
        info() << "\t" << nodeCache.second->currentFlow() << " "
               << nodeCache.second->isFlowFinal() << " " << nodeCache.second->lastModified();
    }

    for (const auto &nodeCache : mCachesNew) {
        info() << "Node address: " << nodeCache.first;
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
