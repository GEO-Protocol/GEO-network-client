#include "TopologyCacheManager.h"

TopologyCacheManager::TopologyCacheManager(
    const SerializedEquivalent equivalent,
    Logger &logger):

    mEquivalent(equivalent),
    mLog(logger)
{
    mInitiatorCache.first = false;
}

void TopologyCacheManager::addCache(
    BaseAddress::Shared keyAddress,
    TopologyCache::Shared cache)
{
    mCaches.insert(
        make_pair(
            keyAddress->fullAddress(),
            cache));
    msCache.insert(
        make_pair(
            utc_now(),
            keyAddress));
}

void TopologyCacheManager::addCacheNew(
    BaseAddress::Shared keyAddress,
    TopologyCacheNew::Shared cache)
{
    mCachesNew.insert(
        make_pair(
            keyAddress->fullAddress(),
            cache));
    msCacheNew.insert(
        make_pair(
            utc_now(),
            keyAddress));
}

TopologyCache::Shared TopologyCacheManager::cacheByAddress(
    BaseAddress::Shared nodeAddress) const
{
    auto nodeAddressAndCache = mCaches.find(nodeAddress->fullAddress());
    if (nodeAddressAndCache == mCaches.end()) {
        return nullptr;
    }
    return nodeAddressAndCache->second;
}

TopologyCacheNew::Shared TopologyCacheManager::cacheByAddressNew(
    BaseAddress::Shared nodeAddress) const
{
    auto nodeAddressAndCache = mCachesNew.find(nodeAddress->fullAddress());
    if (nodeAddressAndCache == mCachesNew.end()) {
        return nullptr;
    }
    return nodeAddressAndCache->second;
}

void TopologyCacheManager::updateCaches()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "updateCaches\t" << "mCaches size: " << mCaches.size();
    info() << "updateCaches\t" << "msCaches size: " << msCache.size();
#endif
    for (auto &timeAndNodeAddress : msCache) {
        if (utc_now() - timeAndNodeAddress.first > kResetSenderCacheDuration()) {
            auto keyAddress = timeAndNodeAddress.second;
#ifdef  DEBUG_LOG_MAX_FLOW_CALCULATION
            info() << "updateCaches delete cache\t" << keyAddress->fullAddress();
#endif
            mCaches.erase(keyAddress->fullAddress());
            msCache.erase(timeAndNodeAddress.first);
        } else {
            break;
        }
    }
    ///////////////////////////////////////////////////////////////////////////////////////////
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "updateCaches\t" << "mCachesNew size: " << mCachesNew.size();
    info() << "updateCaches\t" << "msCachesNew size: " << msCacheNew.size();
#endif
    for (auto &timeAndNodeAddress : msCacheNew) {
        if (utc_now() - timeAndNodeAddress.first > kResetSenderCacheDuration()) {
            auto keyAddress = timeAndNodeAddress.second;
#ifdef  DEBUG_LOG_MAX_FLOW_CALCULATION
            info() << "updateCachesNew delete cache\t" << keyAddress->fullAddress();
#endif
            mCachesNew.erase(keyAddress->fullAddress());
            msCacheNew.erase(timeAndNodeAddress.first);
        } else {
            break;
        }
    }
    /////////////////////////////////////////////////////////////////////////////////////////////
    if (mInitiatorCache.first && utc_now() - mInitiatorCache.second > kResetInitiatorCacheDuration()) {
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
        info() << "updateCaches\t" << "reset Initiator cache";
#endif
        mInitiatorCache.first = false;
    }
}

void TopologyCacheManager::setInitiatorCache()
{
    mInitiatorCache.first = true;
    mInitiatorCache.second = utc_now();
}

void TopologyCacheManager::resetInitiatorCache()
{
    mInitiatorCache.first = false;
}

bool TopologyCacheManager::isInitiatorCached()
{
    return mInitiatorCache.first;
}

void TopologyCacheManager::removeCache(
    BaseAddress::Shared nodeAddress)
{
    for (auto &timeAndNodeAddress : msCache) {
        if (timeAndNodeAddress.second == nodeAddress) {
#ifdef  DEBUG_LOG_MAX_FLOW_CALCULATION
            info() << "removeCache delete cache\t" << timeAndNodeAddress.second->fullAddress();
#endif
            mCaches.erase(timeAndNodeAddress.second->fullAddress());
            msCache.erase(timeAndNodeAddress.first);
            return;
        }
    }
    warning() << "no cache found for key " << nodeAddress->fullAddress();
    //////////////////////////////////////////////////////////////////////////
    for (auto &timeAndNodeAddress : msCacheNew) {
        if (timeAndNodeAddress.second == nodeAddress) {
#ifdef  DEBUG_LOG_MAX_FLOW_CALCULATION
            info() << "removeCacheNew delete cache\t" << timeAndNodeAddress.second->fullAddress();
#endif
            mCachesNew.erase(timeAndNodeAddress.second->fullAddress());
            msCacheNew.erase(timeAndNodeAddress.first);
            return;
        }
    }
    warning() << "no cacheNew found for key " << nodeAddress->fullAddress();
}

DateTime TopologyCacheManager::closestTimeEvent() const
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
    if (!msCacheNew.empty()) {
        auto timeAndNodeAddress = msCacheNew.cbegin();
        if (timeAndNodeAddress->first + kResetSenderCacheDuration() < result) {
            result = timeAndNodeAddress->first + kResetSenderCacheDuration();
        }
    } else {
        if (utc_now() + kResetSenderCacheDuration() < result) {
            result = utc_now() + kResetSenderCacheDuration();
        }
    }
#ifdef  DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "TopologyCacheManager::closestTimeEvent " << result;
#endif
    return result;
}

LoggerStream TopologyCacheManager::info() const
{
    return mLog.info(logHeader());
}

LoggerStream TopologyCacheManager::warning() const
{
    return mLog.warning(logHeader());
}

const string TopologyCacheManager::logHeader() const
{
    stringstream s;
    s << "[TopologyCacheManager: " << mEquivalent << "] ";
    return s.str();
}
