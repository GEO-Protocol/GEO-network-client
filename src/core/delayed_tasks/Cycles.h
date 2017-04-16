#ifndef GEO_NETWORK_CLIENT_CYCLES_H
#define GEO_NETWORK_CLIENT_CYCLES_H

#include <boost/signals2.hpp>
#include <boost/asio.hpp>
#include <iostream>

using namespace std;

namespace as = boost::asio;
namespace signals = boost::signals2;

class CyclesDelayedTasks {
public:
    CyclesDelayedTasks(as::io_service &mIOService);

    signals::signal<void()> mSixNodesCycleSignal;
    signals::signal<void()> mFiveNodesCycleSignal;

    signals::signal<void()> mThreeNodesCycleSignal;
    signals::signal<void()> mFourNodesCycleSignal;

public:
    void RunSignalSixNodes(const boost::system::error_code &error);
    void RunSignalFiveNodes(const boost::system::error_code &error);
    void RunSignalFourNodes(const boost::system::error_code &error);
    void RunSignalThreeNodes(const boost::system::error_code &error);

private:
    as::io_service &mIOService;
    const int mSixNodesSignalRepeatTimeSeconds = 24*60;
    const int mFiveNodesSignalRepeatTimeSeconds = 24*60;

    const int mFourNodesSignalRepeatTimeSeconds = 24*60;
    const int mThreeNodesSignalRepeatTimeSeconds = 24*60;


    unique_ptr<as::deadline_timer> mSixNodesCycleTimer;
    unique_ptr<as::deadline_timer> mFiveNodesCycleTimer;
    unique_ptr<as::deadline_timer> mFourNodesCycleTimer;
    unique_ptr<as::deadline_timer> mThreeNodesCycleTimer;
};

#endif //GEO_NETWORK_CLIENT_CYCLES_H
