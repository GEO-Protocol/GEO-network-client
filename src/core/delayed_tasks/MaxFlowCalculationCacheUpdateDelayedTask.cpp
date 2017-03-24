#include "MaxFlowCalculationCacheUpdateDelayedTask.h"

MaxFlowCalculationCacheUpdateDelayedTask::MaxFlowCalculationCacheUpdateDelayedTask(
    as::io_service &ioService,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheMnager,
    MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
    Logger *logger):

    mIOService(ioService),
    mMaxFlowCalculationCacheMnager(maxFlowCalculationCacheMnager),
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager),
    mLog(logger){

    mMaxFlowCalculationCacheUpdateTimer = unique_ptr<as::steady_timer> (new as::steady_timer(
        mIOService));

    /*mMaxFlowCalculationCacheUpdateTimer = unique_ptr<as::deadline_timer> (new as::deadline_timer(
            mIOService));*/

    GEOEpochTimestamp now = microsecondsSinceGEOEpoch(utc_now());
    GEOEpochTimestamp nextAwakeningTimestamp = microsecondsSinceGEOEpoch(minimalAwakeningTimestamp());
    GEOEpochTimestamp microsecondsDelay = nextAwakeningTimestamp - now;
    mMaxFlowCalculationCacheUpdateTimer->expires_from_now(chrono::microseconds(microsecondsDelay));

#ifdef MAX_FLOW_CALCULATION_DEBUG_LOG
    info() << "constructor at " << utc_now();
    info() << "closest time: " << nextAwakeningTimestamp;
    info() << "microseconds delay: " << chrono::microseconds(microsecondsDelay).count();
#endif

    mMaxFlowCalculationCacheUpdateTimer->async_wait(boost::bind(
        &MaxFlowCalculationCacheUpdateDelayedTask::runSignalMaxFlowCalculationCacheUpdate,
        this,
        as::placeholders::error));
}

void MaxFlowCalculationCacheUpdateDelayedTask::runSignalMaxFlowCalculationCacheUpdate(
    const boost::system::error_code &errorCode) {

    if (errorCode) {
        error() << errorCode.message().c_str();
    }

    updateCache();

    GEOEpochTimestamp now = microsecondsSinceGEOEpoch(utc_now());
    GEOEpochTimestamp nextAwakeningTimestamp = microsecondsSinceGEOEpoch(minimalAwakeningTimestamp());
    GEOEpochTimestamp microsecondsDelay = nextAwakeningTimestamp - now;
    mMaxFlowCalculationCacheUpdateTimer->expires_from_now(chrono::microseconds(microsecondsDelay));

#ifdef MAX_FLOW_CALCULATION_DEBUG_LOG
    info() << "run at " << utc_now();
    info() << "closest time: " << nextAwakeningTimestamp;
    info() << "microseconds delay: " << chrono::microseconds(microsecondsDelay).count();
#endif

    mMaxFlowCalculationCacheUpdateTimer->async_wait(boost::bind(
        &MaxFlowCalculationCacheUpdateDelayedTask::runSignalMaxFlowCalculationCacheUpdate,
        this,
        as::placeholders::error));

    //mMaxFlowCalculationCacheUpdateSignal();
}

DateTime MaxFlowCalculationCacheUpdateDelayedTask::minimalAwakeningTimestamp() {

    DateTime closestCacheManagerTimeEvent = mMaxFlowCalculationCacheMnager->closestTimeEvent();
    DateTime closestTrustLineManagerTimeEvent = mMaxFlowCalculationTrustLineManager->closestTimeEvent();
    if (closestCacheManagerTimeEvent < closestTrustLineManagerTimeEvent) {
        return closestCacheManagerTimeEvent;
    } else {
        return closestTrustLineManagerTimeEvent;
    }
}

void MaxFlowCalculationCacheUpdateDelayedTask::updateCache() {

    mMaxFlowCalculationCacheMnager->updateCaches();
    mMaxFlowCalculationTrustLineManager->deleteLegacyTrustLines();
}

LoggerStream MaxFlowCalculationCacheUpdateDelayedTask::info() const {

    if (nullptr == mLog)
        throw Exception("logger is not initialised");

    return mLog->info(logHeader());
}

LoggerStream MaxFlowCalculationCacheUpdateDelayedTask::error() const {
    if (nullptr == mLog)
        throw Exception("logger is not initialised");

    return mLog->error(logHeader());
}

const string MaxFlowCalculationCacheUpdateDelayedTask::logHeader() const {

    stringstream s;
    s << "[MaxFlowCalculationCacheUpdateDelayedTask]";

    return s.str();
}
