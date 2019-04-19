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

TopologyCache::Shared TopologyCacheManager::cacheByAddress(
    BaseAddress::Shared nodeAddress) const
{
    auto nodeAddressAndCache = mCaches.find(nodeAddress->fullAddress());
    if (nodeAddressAndCache == mCaches.end()) {
        return nullptr;
    }
    return nodeAddressAndCache->second;
}

bool TopologyCacheManager::addIntoFirstLevelCache(
    ContractorID contractorID)
{
    const auto &it = mFirstLvCache.find(contractorID);
    if(it != mFirstLvCache.end()) {
        it->second->get()->second = utc_now();
        mFirstLvCacheList.splice(
            mFirstLvCacheList.end(),
            mFirstLvCacheList,
            it->second);
        mFirstLvCache[contractorID] = mFirstLvCacheList.begin();
        return true;
    }
    mFirstLvCache[contractorID] =
        mFirstLvCacheList.insert(
            mFirstLvCacheList.end(),
            make_shared<FirstLvShared::element_type>(
                make_pair(
                    contractorID,
                    utc_now()
                )
            )
        );
    return true;
}

bool TopologyCacheManager::isInFirstLevelCache(
    ContractorID contractorID) const
{
    return mFirstLvCache.find(contractorID) != mFirstLvCache.end();
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
    /////////////////////////////////////////////////////////////////////////////////////////////
    if (mInitiatorCache.first && utc_now() - mInitiatorCache.second > kResetInitiatorCacheDuration()) {
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
        info() << "updateCaches\t" << "reset Initiator cache";
#endif
        mInitiatorCache.first = false;
    }

    DateTime now = utc_now();
    for(auto current=mFirstLvCacheList.begin(); current!=mFirstLvCacheList.end(); ) {
        auto &cache = *current;
        if ((now - cache.get()->second) <= kResetFirstLvCacheDuration()) {
            break;
        }
        mFirstLvCache.erase(cache.get()->first);
        current = mFirstLvCacheList.erase(current);
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
    if (!msCache.empty()) {
        auto timeAndNodeAddress = msCache.cbegin();
        if (timeAndNodeAddress->first + kResetSenderCacheDuration() < result) {
            result = timeAndNodeAddress->first + kResetSenderCacheDuration();
        }
    } else {
        if (utc_now() + kResetSenderCacheDuration() < result) {
            result = utc_now() + kResetSenderCacheDuration();
        }
    }

    if (!mFirstLvCacheList.empty()) {
        auto nodeIdAndTime = mFirstLvCacheList.cbegin();
        if ((*nodeIdAndTime)->second + kResetFirstLvCacheDuration() < result) {
            result = (*nodeIdAndTime)->second + kResetFirstLvCacheDuration();
        }
    } else {
        if (utc_now() + kResetFirstLvCacheDuration() < result) {
            result = utc_now() + kResetFirstLvCacheDuration();
        }
    }
#ifdef  DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "closestTimeEvent " << result;
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
