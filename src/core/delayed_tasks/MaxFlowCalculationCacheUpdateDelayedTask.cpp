#include "MaxFlowCalculationCacheUpdateDelayedTask.h"

MaxFlowCalculationCacheUpdateDelayedTask::MaxFlowCalculationCacheUpdateDelayedTask(
    as::io_service &ioService):
    mIOService(ioService){
    mMaxFlowCalculationCacheUpdateTimer = unique_ptr<as::deadline_timer> (new as::deadline_timer(
        mIOService));

    mMaxFlowCalculationCacheUpdateTimer->expires_from_now(boost::posix_time::seconds(kSignalRepeatTimeSeconds));
    mMaxFlowCalculationCacheUpdateTimer->async_wait(boost::bind(
        &MaxFlowCalculationCacheUpdateDelayedTask::runSignalMaxFlowCalculationCacheUpdate,
        this,
        as::placeholders::error));
}

void MaxFlowCalculationCacheUpdateDelayedTask::runSignalMaxFlowCalculationCacheUpdate(
    const boost::system::error_code &error) {

    if (error) {
        cout << error.message() << endl;
    }

    mMaxFlowCalculationCacheUpdateTimer->expires_from_now(boost::posix_time::seconds(kSignalRepeatTimeSeconds));
    mMaxFlowCalculationCacheUpdateTimer->async_wait(boost::bind(
        &MaxFlowCalculationCacheUpdateDelayedTask::runSignalMaxFlowCalculationCacheUpdate,
        this,
        as::placeholders::error));

    mMaxFlowCalculationCacheUpdateSignal();
}
