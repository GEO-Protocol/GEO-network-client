#include "TopologyCacheUpdateDelayedTask.h"

TopologyCacheUpdateDelayedTask::TopologyCacheUpdateDelayedTask(
    const SerializedEquivalent equivalent,
    as::io_service &ioService,
    TopologyCacheManager *topologyCacheManager,
    TopologyTrustLinesManager *topologyTrustLineManager,
    MaxFlowCacheManager *maxFlowCalculationNodeCacheManager,
    Logger &logger):

    mEquivalent(equivalent),
    mIOService(ioService),
    mTopologyCacheManager(topologyCacheManager),
    mTopologyTrustLineManager(topologyTrustLineManager),
    mMaxFlowCalculationNodeCacheManager(maxFlowCalculationNodeCacheManager),
    mLog(logger)
{
    mTopologyCacheUpdateTimer = make_unique<as::steady_timer>(
        mIOService);

    Duration microsecondsDelay = minimalAwakeningTimestamp() - utc_now();
    mTopologyCacheUpdateTimer->expires_from_now(
        chrono::milliseconds(
            microsecondsDelay.total_milliseconds()));
    mTopologyCacheUpdateTimer->async_wait(boost::bind(
            &TopologyCacheUpdateDelayedTask::runSignalTopologyCacheUpdate,
        this,
        as::placeholders::error));
}

void TopologyCacheUpdateDelayedTask::runSignalTopologyCacheUpdate(
        const boost::system::error_code &errorCode)
{
    if (errorCode) {
        warning() << errorCode.message().c_str();
    }
    DateTime closestTimeEvent = updateCache();
    Duration microsecondsDelay = closestTimeEvent - utc_now();
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    auto duration = chrono::milliseconds(microsecondsDelay.total_milliseconds());
    debug() << "next launch: " << duration.count() << " ms" << endl;
#endif
    mTopologyCacheUpdateTimer->expires_from_now(
        chrono::milliseconds(
            microsecondsDelay.total_milliseconds()));
    mTopologyCacheUpdateTimer->async_wait(boost::bind(
            &TopologyCacheUpdateDelayedTask::runSignalTopologyCacheUpdate,
        this,
        as::placeholders::error));
    mTopologyTrustLineManager->setPreventDeleting(false);
}

DateTime TopologyCacheUpdateDelayedTask::minimalAwakeningTimestamp()
{
    DateTime closestCacheManagerTimeEvent = mTopologyCacheManager->closestTimeEvent();
    DateTime closestTrustLineManagerTimeEvent = mTopologyTrustLineManager->closestTimeEvent();
    DateTime closestNodeCacheManagerTimeEvent = mMaxFlowCalculationNodeCacheManager->closestTimeEvent();
    DateTime result = closestCacheManagerTimeEvent;
    if (closestTrustLineManagerTimeEvent < result) {
        result = closestTrustLineManagerTimeEvent;
    }
    if (closestNodeCacheManagerTimeEvent < result) {
        result = closestNodeCacheManagerTimeEvent;
    }
    return result;
}

DateTime TopologyCacheUpdateDelayedTask::updateCache()
{
    mTopologyCacheManager->updateCaches();
    DateTime result = mTopologyCacheManager->closestTimeEvent();
    mMaxFlowCalculationNodeCacheManager->updateCaches();
    DateTime closestNodeCacheManagerTimeEvent = mMaxFlowCalculationNodeCacheManager->closestTimeEvent();
    if (closestNodeCacheManagerTimeEvent < result) {
        result = closestNodeCacheManagerTimeEvent;
    }
    // if MaxFlowCalculation transaction not finished it is forbidden to delete trustlines
    // and should increse trustlines caches time
    DateTime closestTrustLineManagerTimeEvent;
    if (mTopologyTrustLineManager->preventDeleting()) {
        closestTrustLineManagerTimeEvent = utc_now() + kProlongationTrustLineUpdatingDuration();
    } else {
        // if at least one trustline was deleted
        if (mTopologyTrustLineManager->deleteLegacyTrustLines()) {
            mTopologyCacheManager->resetInitiatorCache();
        }
        closestTrustLineManagerTimeEvent = mTopologyTrustLineManager->closestTimeEvent();
    }
    if (closestTrustLineManagerTimeEvent < result) {
        result = closestTrustLineManagerTimeEvent;
    }
    return result;
}

LoggerStream TopologyCacheUpdateDelayedTask::debug() const
{
    return mLog.debug(logHeader());
}

LoggerStream TopologyCacheUpdateDelayedTask::info() const
{
    return mLog.info(logHeader());
}

LoggerStream TopologyCacheUpdateDelayedTask::warning() const
{
    return mLog.warning(logHeader());
}

const string TopologyCacheUpdateDelayedTask::logHeader() const
{
    stringstream s;
    s << "[TopologyCacheUpdateDelayedTask: " << mEquivalent << "] ";
    return s.str();
}
