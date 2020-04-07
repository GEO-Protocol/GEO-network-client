#ifndef GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONANDROUTINGTABLESDELAYEDTASK_H
#define GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONANDROUTINGTABLESDELAYEDTASK_H

#include "../common/time/TimeUtils.h"
#include "../logger/Logger.h"
#include "../transactions/transactions/base/TransactionUUID.h"

#include <boost/asio/steady_timer.hpp>
#include <boost/signals2.hpp>
#include <boost/asio.hpp>

using namespace std;
namespace as = boost::asio;
namespace signals = boost::signals2;

class GatewayNotificationAndRoutingTablesDelayedTask {

public:
    typedef signals::signal<void()> GatewayNotificationSignal;

public:
    GatewayNotificationAndRoutingTablesDelayedTask(
        bool enabled,
        uint32_t updatingTimerPeriodSeconds,
        as::io_service &ioService,
        Logger &logger);

public:
    mutable GatewayNotificationSignal gatewayNotificationSignal;

private:
    void runSignalNotify(
        const boost::system::error_code &error);

    uint32_t randomInitializer() const;

    LoggerStream info() const;

    LoggerStream debug() const;

    LoggerStream warning() const;

    const string logHeader() const;

private:
    uint32_t mUpdatingTimerPeriodDays;
    as::io_service &mIOService;
    unique_ptr<as::steady_timer> mNotificationTimer;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONANDROUTINGTABLESDELAYEDTASK_H
