#include "MaxFlowCacheManager.h"

MaxFlowCacheManager::MaxFlowCacheManager(
    const SerializedEquivalent equivalent,
    Logger &logger):

    mEquivalent(equivalent),
    mLog(logger)
{}

void MaxFlowCacheManager::addCache(
        BaseAddress::Shared keyAddress,
        MaxFlowCache::Shared cache)
{
    mCaches.insert(
        make_pair(
            keyAddress->fullAddress(),
            cache));
    mTimeCaches.insert(
        make_pair(
            utc_now(),
            keyAddress));
}

void MaxFlowCacheManager::updateCaches()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "updateCaches\t" << "mCaches size: " << mCaches.size();
    info() << "updateCaches\t" << "msCaches size: " << mTimeCaches.size();
#endif
    for (auto &timeAndNodeAddress : mTimeCaches) {
        if (utc_now() - timeAndNodeAddress.first > kResetCacheDuration()) {
            auto keyAddressPtr = timeAndNodeAddress.second;
#ifdef  DEBUG_LOG_MAX_FLOW_CALCULATION
            info() << "updateCaches delete cache\t" << keyAddressPtr->fullAddress();
#endif
            mCaches.erase(keyAddressPtr->fullAddress());
            mTimeCaches.erase(timeAndNodeAddress.first);
        } else {
            break;
        }
    }
}

MaxFlowCache::Shared MaxFlowCacheManager::cacheByAddress(
    BaseAddress::Shared nodeAddress) const
{
    auto nodeAddressAndCache = mCaches.find(nodeAddress->fullAddress());
    if (nodeAddressAndCache == mCaches.end()) {
        return nullptr;
    }
    return nodeAddressAndCache->second;
}

void MaxFlowCacheManager::updateCache(
    BaseAddress::Shared keyAddress,
    const TrustLineAmount &amount,
    bool isFinal)
{
    auto nodeAddressAndCache = mCaches.find(keyAddress->fullAddress());
    if (nodeAddressAndCache == mCaches.end()) {
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
    if (!mTimeCaches.empty()) {
        auto timeAndNodeAddress = mTimeCaches.cbegin();
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
    mTimeCaches.clear();
    mCaches.clear();
}

void MaxFlowCacheManager::printCaches()
{
    for (const auto &nodeCache : mCaches) {
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
