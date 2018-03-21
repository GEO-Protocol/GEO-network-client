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
    const NodeUUID &keyUUID,
    TopologyCache::Shared cache)
{
    auto nodeUUIDPtr = new NodeUUID(keyUUID);
    mCaches.insert(
        make_pair(
            *nodeUUIDPtr,
            cache));
    msCache.insert(
        make_pair(
            utc_now(),
            nodeUUIDPtr));
}

TopologyCache::Shared TopologyCacheManager::cacheByNode(
    const NodeUUID &nodeUUID) const
{
    auto nodeUUIDAndCache = mCaches.find(nodeUUID);
    if (nodeUUIDAndCache == mCaches.end()) {
        return nullptr;
    }
    return nodeUUIDAndCache->second;
}

void TopologyCacheManager::updateCaches()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "updateCaches\t" << "mCaches size: " << mCaches.size();
    info() << "updateCaches\t" << "msCaches size: " << msCache.size();
#endif
    for (auto &timeAndNodeUUID : msCache) {
        if (utc_now() - timeAndNodeUUID.first > kResetSenderCacheDuration()) {
            NodeUUID* keyUUIDPtr = timeAndNodeUUID.second;
#ifdef  DEBUG_LOG_MAX_FLOW_CALCULATION
            info() << "updateCaches delete cache\t" << *keyUUIDPtr;
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
    const NodeUUID &nodeUUID)
{
    for (auto &timeAndNodeUUID : msCache) {
        NodeUUID* keyUUIDPtr = timeAndNodeUUID.second;
        if (*keyUUIDPtr == nodeUUID) {
#ifdef  DEBUG_LOG_MAX_FLOW_CALCULATION
            info() << "removeCache delete cache\t" << *keyUUIDPtr;
#endif
            mCaches.erase(*keyUUIDPtr);
            msCache.erase(timeAndNodeUUID.first);
            delete keyUUIDPtr;
            return;
        }
    }
    warning() << "no cache found for key " << nodeUUID;
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
