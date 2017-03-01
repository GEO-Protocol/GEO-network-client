#include "MaxFlowCalculationCacheUpdateDelayedTask.h"

MaxFlowCalculationCacheUpdateDelayedTask::MaxFlowCalculationCacheUpdateDelayedTask(
    as::io_service &ioService):
    mIOService(ioService){
    /*mMaxFlowCalculationCacheUpdateTimer = unique_ptr<as::deadline_timer> (new as::deadline_timer(
        mIOService,
        boost::posix_time::seconds(5)));
    mMaxFlowCalculationCacheUpdateTimer->expires_from_now(boost::posix_time::seconds(mSignalRepeatTimeSeconds));
    mMaxFlowCalculationCacheUpdateTimer->async_wait(boost::bind(
        &MaxFlowCalculationCacheUpdateDelayedTask::runSignalMaxFlowCalculationCacheUpdate,
        this,
        as::placeholders::error));
    cout << "MaxFlowCalculationCacheUpdateDelayedTask constructor\n";*/
}

void MaxFlowCalculationCacheUpdateDelayedTask::runSignalMaxFlowCalculationCacheUpdate(
    const boost::system::error_code &error) {

    if (error) {
        cout << error.message() << endl;
    }
    //mMaxFlowCalculationCacheUpdateTimer->cancel();
    //mMaxFlowCalculationCacheUpdateTimer->expires_from_now(boost::posix_time::seconds(mSignalRepeatTimeSeconds));
    /*mMaxFlowCalculationCacheUpdateTimer->async_wait(boost::bind(
        &MaxFlowCalculationCacheUpdateDelayedTask::runSignalMaxFlowCalculationCacheUpdate,
        this,
        as::placeholders::error));*/
    cout << "runSignalMaxFlowCalculationCacheUpdate\n";
    mMaxFlowCalculationCacheUpdateSignal();
    cout << "runSignalMaxFlowCalculationCacheUpdate after signal\n";
}
