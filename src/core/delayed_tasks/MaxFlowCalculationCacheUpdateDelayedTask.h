#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHEUPDATEDELAYEDTASK_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHEUPDATEDELAYEDTASK_H

#include <boost/signals2.hpp>
#include <boost/asio.hpp>
#include <iostream>

using namespace std;

namespace as = boost::asio;
namespace signals = boost::signals2;

class MaxFlowCalculationCacheUpdateDelayedTask {

public:
    MaxFlowCalculationCacheUpdateDelayedTask(as::io_service &mIOService);

public:
    signals::signal<void()> mMaxFlowCalculationCacheUpdateSignal;

public:
    void runSignalMaxFlowCalculationCacheUpdate(
        const boost::system::error_code &error);

private:
    as::io_service &mIOService;
    const int mSignalRepeatTimeSeconds = 10;

    unique_ptr<as::deadline_timer> mMaxFlowCalculationCacheUpdateTimer;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHEUPDATEDELAYEDTASK_H
