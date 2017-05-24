#include "Cycles.h"

void CyclesDelayedTasks::RunSignalFiveNodes(const boost::system::error_code &error) {
    if (error) {
        cout << error.message() << endl;
    }
    mFiveNodesCycleTimer->cancel();
    mFiveNodesCycleTimer->expires_from_now(std::chrono::seconds(mFiveNodesSignalRepeatTimeSeconds));
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
    mSixNodesCycleTimer->expires_from_now(std::chrono::seconds(mSixNodesSignalRepeatTimeSeconds));
    mSixNodesCycleTimer->async_wait(boost::bind(
            &CyclesDelayedTasks::RunSignalSixNodes,
            this,
            as::placeholders::error
    ));
    mSixNodesCycleSignal();
}

CyclesDelayedTasks::CyclesDelayedTasks(as::io_service &ioService):mIOService(ioService){
//    todo add set Time started to 60*6
    srand(time(NULL));
    int TimeStarted = rand() % 60 * 60 * 6;
    #ifdef TESTS
    TimeStarted = 120;
    #endif
    mFiveNodesCycleTimer = make_unique<as::steady_timer>(
        mIOService);
    mFiveNodesCycleTimer->expires_from_now(std::chrono::seconds(TimeStarted));
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
    mSixNodesCycleTimer = make_unique<as::steady_timer>(
        mIOService);
    mSixNodesCycleTimer->expires_from_now(std::chrono::seconds(TimeStarted));
    mSixNodesCycleTimer->async_wait(boost::bind(
            &CyclesDelayedTasks::RunSignalSixNodes,
            this,
            as::placeholders::error
    ));

    #ifdef TESTS
    TimeStarted = 120;
    mFourNodesCycleTimer = make_unique<as::steady_timer>(
        mIOService);
    mFourNodesCycleTimer->expires_from_now(std::chrono::seconds(TimeStarted));
    mFourNodesCycleTimer->async_wait(boost::bind(
            &CyclesDelayedTasks::RunSignalFourNodes,
            this,
            as::placeholders::error
    ));
    TimeStarted = 120;
    mThreeNodesCycleTimer = make_unique<as::steady_timer>(
        mIOService);
    mThreeNodesCycleTimer->expires_from_now(std::chrono::seconds(TimeStarted));
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
    mFourNodesCycleTimer->expires_from_now(std::chrono::seconds(mFourNodesSignalRepeatTimeSeconds));
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
    mThreeNodesCycleTimer->expires_from_now(std::chrono::seconds(mThreeNodesSignalRepeatTimeSeconds));
    mThreeNodesCycleTimer->async_wait(boost::bind(
            &CyclesDelayedTasks::RunSignalThreeNodes,
            this,
            as::placeholders::error
    ));
    mThreeNodesCycleSignal();
}
