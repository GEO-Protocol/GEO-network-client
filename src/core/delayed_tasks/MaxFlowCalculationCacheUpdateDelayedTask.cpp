#include "MaxFlowCalculationCacheUpdateDelayedTask.h"

MaxFlowCalculationCacheUpdateDelayedTask::MaxFlowCalculationCacheUpdateDelayedTask(
    as::io_service &ioService,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheMnager,
    MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
    MaxFlowCalculationNodeCacheManager *maxFlowCalculationNodeCacheManager,
    Logger &logger):

    mIOService(ioService),
    mMaxFlowCalculationCacheMnager(maxFlowCalculationCacheMnager),
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager),
    mMaxFlowCalculationNodeCacheManager(maxFlowCalculationNodeCacheManager),
    mLog(logger)
{
    mMaxFlowCalculationCacheUpdateTimer = make_unique<as::steady_timer>(
        mIOService);

    Duration microsecondsDelay = minimalAwakeningTimestamp() - utc_now();
    mMaxFlowCalculationCacheUpdateTimer->expires_from_now(
        chrono::milliseconds(
            microsecondsDelay.total_milliseconds()));
    mMaxFlowCalculationCacheUpdateTimer->async_wait(boost::bind(
        &MaxFlowCalculationCacheUpdateDelayedTask::runSignalMaxFlowCalculationCacheUpdate,
        this,
        as::placeholders::error));
}

void MaxFlowCalculationCacheUpdateDelayedTask::runSignalMaxFlowCalculationCacheUpdate(
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
    mMaxFlowCalculationCacheUpdateTimer->expires_from_now(
        chrono::milliseconds(
            microsecondsDelay.total_milliseconds()));
    mMaxFlowCalculationCacheUpdateTimer->async_wait(boost::bind(
        &MaxFlowCalculationCacheUpdateDelayedTask::runSignalMaxFlowCalculationCacheUpdate,
        this,
        as::placeholders::error));
    mMaxFlowCalculationTrustLineManager->setPreventDeleting(false);
}

DateTime MaxFlowCalculationCacheUpdateDelayedTask::minimalAwakeningTimestamp()
{
    DateTime closestCacheManagerTimeEvent = mMaxFlowCalculationCacheMnager->closestTimeEvent();
    DateTime closestTrustLineManagerTimeEvent = mMaxFlowCalculationTrustLineManager->closestTimeEvent();
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

DateTime MaxFlowCalculationCacheUpdateDelayedTask::updateCache()
{
    mMaxFlowCalculationCacheMnager->updateCaches();
    DateTime result = mMaxFlowCalculationCacheMnager->closestTimeEvent();
    mMaxFlowCalculationNodeCacheManager->updateCaches();
    DateTime closestNodeCacheManagerTimeEvent = mMaxFlowCalculationNodeCacheManager->closestTimeEvent();
    if (closestNodeCacheManagerTimeEvent < result) {
        result = closestNodeCacheManagerTimeEvent;
    }
    // if MaxFlowCalculation transaction not finished it is forbidden to delete trustlines
    // and should increse trustlines caches time
    DateTime closestTrustLineManagerTimeEvent;
    if (mMaxFlowCalculationTrustLineManager->preventDeleting()) {
        closestTrustLineManagerTimeEvent = utc_now() + kProlongationTrustLineUpdatingDuration();
    } else {
        // if at least one trustline was deleted
        if (mMaxFlowCalculationTrustLineManager->deleteLegacyTrustLines()) {
            mMaxFlowCalculationCacheMnager->resetInitiatorCache();
        }
        closestTrustLineManagerTimeEvent = mMaxFlowCalculationTrustLineManager->closestTimeEvent();
    }
    if (closestTrustLineManagerTimeEvent < result) {
        result = closestTrustLineManagerTimeEvent;
    }
    return result;
}

LoggerStream MaxFlowCalculationCacheUpdateDelayedTask::debug() const
{
    return mLog.debug(logHeader());
}

LoggerStream MaxFlowCalculationCacheUpdateDelayedTask::info() const
{
    return mLog.info(logHeader());
}

LoggerStream MaxFlowCalculationCacheUpdateDelayedTask::warning() const
{
    return mLog.warning(logHeader());
}

const string MaxFlowCalculationCacheUpdateDelayedTask::logHeader() const
{
    return "[MaxFlowCalculationCacheUpdateDelayedTask]";
}
