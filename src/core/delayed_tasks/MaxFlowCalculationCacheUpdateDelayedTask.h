#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHEUPDATEDELAYEDTASK_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHEUPDATEDELAYEDTASK_H

#include "../max_flow_calculation/cashe/MaxFlowCalculationCacheManager.h"
#include "../max_flow_calculation/manager/MaxFlowCalculationTrustLineManager.h"
#include "../max_flow_calculation/cashe/MaxFlowCalculationNodeCacheManager.h"
#include "../common/time/TimeUtils.h"
#include "../logger/Logger.h"

#include <boost/asio/steady_timer.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/signals2.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <chrono>

using namespace std;

namespace as = boost::asio;

class MaxFlowCalculationCacheUpdateDelayedTask {

public:
    MaxFlowCalculationCacheUpdateDelayedTask(
        as::io_service &mIOService,
        MaxFlowCalculationCacheManager *maxflowCalculationCacheMnager,
        MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
        MaxFlowCalculationNodeCacheManager *maxFlowCalculationNodeCacheManager,
        Logger &logger);

public:
    void runSignalMaxFlowCalculationCacheUpdate(
        const boost::system::error_code &error);

private:
    DateTime minimalAwakeningTimestamp();

    // return time when task shoulb be started again
    DateTime updateCache();

    LoggerStream info() const;

    LoggerStream debug() const;

    LoggerStream warning() const;

    const string logHeader() const;

private:
    static const uint16_t kProlongationTrustLineUpdatingTimeSec = 2;

private:
    static Duration& kProlongationTrustLineUpdatingDuration() {
        static auto duration = Duration(
            0,
            0,
            kProlongationTrustLineUpdatingTimeSec);
        return duration;
    }

private:
    as::io_service &mIOService;
    unique_ptr<as::steady_timer> mMaxFlowCalculationCacheUpdateTimer;
    MaxFlowCalculationCacheManager *mMaxFlowCalculationCacheMnager;
    MaxFlowCalculationTrustLineManager *mMaxFlowCalculationTrustLineManager;
    MaxFlowCalculationNodeCacheManager *mMaxFlowCalculationNodeCacheManager;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHEUPDATEDELAYEDTASK_H
