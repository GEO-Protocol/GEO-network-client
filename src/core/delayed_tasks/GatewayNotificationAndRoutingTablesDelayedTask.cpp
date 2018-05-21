#include "GatewayNotificationAndRoutingTablesDelayedTask.h"

GatewayNotificationAndRoutingTablesDelayedTask::GatewayNotificationAndRoutingTablesDelayedTask(
    as::io_service &ioService,
    Logger &logger):

    mIOService(ioService),
    mLog(logger)
{
    mNotificationTimer = make_unique<as::steady_timer>(
        mIOService);
    // todo : rand() used for concurrent start of all nodes or some part of nodes (data center)
    // on decentralize network it is not necessary
    int timeStarted = 120 + rand() % (240);
#ifdef TESTS
    timeStarted = 10;
#endif
    mNotificationTimer->expires_from_now(
        chrono::seconds(
            timeStarted));
    mNotificationTimer->async_wait(
        boost::bind(
            &GatewayNotificationAndRoutingTablesDelayedTask::runSignalNotify,
            this,
            as::placeholders::error));
}

void GatewayNotificationAndRoutingTablesDelayedTask::runSignalNotify(
    const boost::system::error_code &errorCode)
{
    if (errorCode) {
        warning() << errorCode.message().c_str();
    }
    info() << "run gateway notification signal";
    mNotificationTimer->cancel();
    mNotificationTimer->expires_from_now(
        std::chrono::seconds(
            kUpdatingTimerPeriodSeconds + rand() % (60 * 60 * 24)));
#ifdef TESTS
    mNotificationTimer->expires_from_now(
        std::chrono::seconds(15));
#endif
    mNotificationTimer->async_wait(
        boost::bind(
            &GatewayNotificationAndRoutingTablesDelayedTask::runSignalNotify,
            this,
            as::placeholders::error));
    gatewayNotificationSignal();
}

LoggerStream GatewayNotificationAndRoutingTablesDelayedTask::debug() const
{
    return mLog.debug(logHeader());
}

LoggerStream GatewayNotificationAndRoutingTablesDelayedTask::info() const
{
    return mLog.info(logHeader());
}

LoggerStream GatewayNotificationAndRoutingTablesDelayedTask::warning() const
{
    return mLog.warning(logHeader());
}

const string GatewayNotificationAndRoutingTablesDelayedTask::logHeader() const
{
    stringstream s;
    s << "[GatewayNotificationAndRoutingTablesDelayedTask]";
    return s.str();
}