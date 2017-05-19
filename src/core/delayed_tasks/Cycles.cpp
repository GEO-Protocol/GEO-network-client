#include "Cycles.h"

void CyclesDelayedTasks::RunSignalFiveNodes(const boost::system::error_code &error) {
    if (error) {
        cout << error.message() << endl;
    }
    mFiveNodesCycleTimer->cancel();
    mFiveNodesCycleTimer->expires_from_now(boost::posix_time::seconds(mFiveNodesSignalRepeatTimeSeconds));
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
    mSixNodesCycleTimer->expires_from_now(boost::posix_time::seconds(mSixNodesSignalRepeatTimeSeconds));
    mSixNodesCycleTimer->async_wait(boost::bind(
            &CyclesDelayedTasks::RunSignalSixNodes,
            this,
            as::placeholders::error
    ));
    mSixNodesCycleSignal();
}

CyclesDelayedTasks::CyclesDelayedTasks(as::io_service &ioService):mIOService(ioService){
//    todo add set Time started to 60*6
    int TimeStarted = rand() % 60 *60 * 6;
    #ifdef TESTS
    TimeStarted = 120;
    #endif
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
    TimeStarted = rand() % 60 * 60 * 6;
    #ifdef TESTS
        TimeStarted = 120;
    #endif
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

    #ifdef TESTS
    TimeStarted = 120;
    mFourNodesCycleTimer = unique_ptr<as::deadline_timer> (new as::deadline_timer(
            mIOService,
            boost::posix_time::seconds(5)
    ));
    mFourNodesCycleTimer->expires_from_now(boost::posix_time::seconds(TimeStarted));
    mFourNodesCycleTimer->async_wait(boost::bind(
            &CyclesDelayedTasks::RunSignalFourNodes,
            this,
            as::placeholders::error
    ));
    TimeStarted = 120;
    mThreeNodesCycleTimer = unique_ptr<as::deadline_timer> (new as::deadline_timer(
            mIOService,
            boost::posix_time::seconds(5)
    ));
    mThreeNodesCycleTimer->expires_from_now(boost::posix_time::seconds(TimeStarted));
    mThreeNodesCycleTimer->async_wait(boost::bind(
            &CyclesDelayedTasks::RunSignalThreeNodes,
            this,
            as::placeholders::error
    ));
     #endif

}

void CyclesDelayedTasks::RunSignalFourNodes(const boost::system::error_code &error) {
    if (error) {
        cout << error.message() << endl;
    }
    mFourNodesCycleTimer->cancel();
    mFourNodesCycleTimer->expires_from_now(boost::posix_time::seconds(mFourNodesSignalRepeatTimeSeconds));
    mFourNodesCycleTimer->async_wait(boost::bind(
            &CyclesDelayedTasks::RunSignalFourNodes,
            this,
            as::placeholders::error
    ));
    mFourNodesCycleSignal();
}

void CyclesDelayedTasks::RunSignalThreeNodes(const boost::system::error_code &error) {
    if (error) {
        cout << error.message() << endl;
    }
    mThreeNodesCycleTimer->cancel();
    mThreeNodesCycleTimer->expires_from_now(boost::posix_time::seconds(mThreeNodesSignalRepeatTimeSeconds));
    mThreeNodesCycleTimer->async_wait(boost::bind(
            &CyclesDelayedTasks::RunSignalThreeNodes,
            this,
            as::placeholders::error
    ));
    mThreeNodesCycleSignal();
}
