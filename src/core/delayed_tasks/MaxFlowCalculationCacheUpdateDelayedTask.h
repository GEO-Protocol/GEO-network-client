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
    const uint32_t kSignalRepeatTimeSeconds = 600;

private:
    as::io_service &mIOService;
    unique_ptr<as::deadline_timer> mMaxFlowCalculationCacheUpdateTimer;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHEUPDATEDELAYEDTASK_H
