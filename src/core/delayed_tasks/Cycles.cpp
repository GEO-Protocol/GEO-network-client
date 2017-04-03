#include "Cycles.h"

void CyclesDelayedTasks::RunSignalFiveNodes(const boost::system::error_code &error) {
    if (error) {
        cout << error.message() << endl;
    }
    mFiveNodesCycleTimer->cancel();
    mFiveNodesCycleTimer->expires_from_now(boost::posix_time::seconds(mSixNodesSignalRepeatTimeSeconds));
    mFiveNodesCycleTimer->async_wait(boost::bind(
            &CyclesDelayedTasks::RunSignalFiveNodes,
            this,
            as::placeholders::error
    ));
    mFiveNodesCycleSignal();
}

void CyclesDelayedTasks::RunSignalSixNodes(const boost::system::error_code &error) {
    if (error) {
        cout << error.message() << endl;
    }
    mSixNodesCycleTimer->cancel();
    mSixNodesCycleTimer->expires_from_now(boost::posix_time::seconds(mFiveNodesSignalRepeatTimeSeconds));
    mSixNodesCycleTimer->async_wait(boost::bind(
            &CyclesDelayedTasks::RunSignalSixNodes,
            this,
            as::placeholders::error
    ));
    mSixNodesCycleSignal();
}

CyclesDelayedTasks::CyclesDelayedTasks(as::io_service &ioService):mIOService(ioService){
//    todo add set Time started to 60*6
    int TimeStarted = rand() % 60 * 6;
    mFiveNodesCycleTimer = unique_ptr<as::deadline_timer> (new as::deadline_timer(
            mIOService,
            boost::posix_time::seconds(5)
    ));
    mFiveNodesCycleTimer->expires_from_now(boost::posix_time::seconds(TimeStarted));
    mFiveNodesCycleTimer->async_wait(boost::bind(
            &CyclesDelayedTasks::RunSignalFiveNodes,
            this,
            as::placeholders::error
    ));
    //    todo add set Time started to 60*6
    TimeStarted = rand() % 60 * 6;
    mSixNodesCycleTimer = unique_ptr<as::deadline_timer> (new as::deadline_timer(
            mIOService,
            boost::posix_time::seconds(5)
    ));
    mSixNodesCycleTimer->expires_from_now(boost::posix_time::seconds(TimeStarted));
    mSixNodesCycleTimer->async_wait(boost::bind(
            &CyclesDelayedTasks::RunSignalSixNodes,
            this,
            as::placeholders::error
    ));
}
