#include "MaxFlowCalculationCacheUpdateDelayedTask.h"

MaxFlowCalculationCacheUpdateDelayedTask::MaxFlowCalculationCacheUpdateDelayedTask(
    as::io_service &ioService,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheMnager,
    MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
    Logger &logger):

    mIOService(ioService),
    mMaxFlowCalculationCacheMnager(maxFlowCalculationCacheMnager),
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager),
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
        error() << errorCode.message().c_str();
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
    if (closestCacheManagerTimeEvent < closestTrustLineManagerTimeEvent) {
        return closestCacheManagerTimeEvent;
    } else {
        return closestTrustLineManagerTimeEvent;
    }
}

DateTime MaxFlowCalculationCacheUpdateDelayedTask::updateCache()
{
    mMaxFlowCalculationCacheMnager->updateCaches();
    DateTime closestCacheManagerTimeEvent = mMaxFlowCalculationCacheMnager->closestTimeEvent();
    // if MaxFlowCalculation transaction not finished it is forbidden to delete trustlines
    // and should increse trustlines caches time
    DateTime closestTrustLineManagerTimeEvent;
    if (mMaxFlowCalculationTrustLineManager->preventDeleting()) {
        closestTrustLineManagerTimeEvent = utc_now() + kProlongationTrustLineUpdatingDuration();
    } else {
        // if at least one trustline was deleted
        if (mMaxFlowCalculationTrustLineManager->deleteLegacyTrustLines()) {
            mMaxFlowCalculationCacheMnager->resetInititorCache();
        }
        closestTrustLineManagerTimeEvent = mMaxFlowCalculationTrustLineManager->closestTimeEvent();
    }
    if (closestCacheManagerTimeEvent < closestTrustLineManagerTimeEvent) {
        return closestCacheManagerTimeEvent;
    } else {
        return closestTrustLineManagerTimeEvent;
    }
}

LoggerStream MaxFlowCalculationCacheUpdateDelayedTask::debug() const
{
    return mLog.debug(logHeader());
}

LoggerStream MaxFlowCalculationCacheUpdateDelayedTask::info() const
{
    return mLog.info(logHeader());
}

LoggerStream MaxFlowCalculationCacheUpdateDelayedTask::error() const
{
    return mLog.error(logHeader());
}

const string MaxFlowCalculationCacheUpdateDelayedTask::logHeader() const
{
    return "[MaxFlowCalculationCacheUpdateDelayedTask]";
}
