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

public:
    void RunSignalSixNodes(const boost::system::error_code &error);
    void RunSignalFiveNodes(const boost::system::error_code &error);

private:
    as::io_service &mIOService;
    const int mSignalRepeatTimeSeconds = 10;
    const int mSignalRepeatTimeSeconds1 = 15;


    unique_ptr<as::deadline_timer> mSixNodesCycleTimer;
    unique_ptr<as::deadline_timer> mFiveNodesCycleTimer;
};

#endif //GEO_NETWORK_CLIENT_CYCLES_H
