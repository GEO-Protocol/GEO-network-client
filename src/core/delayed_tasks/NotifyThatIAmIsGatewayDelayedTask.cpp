#include "NotifyThatIAmIsGatewayDelayedTask.h"

NotifyThatIAmIsGatewayDelayedTask::NotifyThatIAmIsGatewayDelayedTask(
    as::io_service &ioService,
    Logger &logger):

    mIOService(ioService),
    mLog(logger)
{
    mNotificationTimer = make_unique<as::steady_timer>(
        mIOService);
    int timeStarted = 120 + rand() % (60);
#ifdef TESTS
    timeStarted = 10;
#endif
    mNotificationTimer->expires_from_now(
        chrono::seconds(
            timeStarted));
    mNotificationTimer->async_wait(
        boost::bind(
            &NotifyThatIAmIsGatewayDelayedTask::runSignalNotify,
            this,
            as::placeholders::error));
}

void NotifyThatIAmIsGatewayDelayedTask::runSignalNotify(
    const boost::system::error_code &errorCode)
{
    if (errorCode) {
        warning() << errorCode.message().c_str();
    }
    info() << "run gateway notification signal";
    mNotificationTimer->cancel();
    mNotificationTimer->expires_from_now(
        std::chrono::seconds(
            kUpdatingTimerPeriodSeconds));
    mNotificationTimer->async_wait(
        boost::bind(
            &NotifyThatIAmIsGatewayDelayedTask::runSignalNotify,
            this,
            as::placeholders::error));
    gatewayNotificationSignal();
}

LoggerStream NotifyThatIAmIsGatewayDelayedTask::debug() const
{
    return mLog.debug(logHeader());
}

LoggerStream NotifyThatIAmIsGatewayDelayedTask::info() const
{
    return mLog.info(logHeader());
}

LoggerStream NotifyThatIAmIsGatewayDelayedTask::warning() const
{
    return mLog.warning(logHeader());
}

const string NotifyThatIAmIsGatewayDelayedTask::logHeader() const
{
    stringstream s;
    s << "[NotifyThatIAmIsGatewayDelayedTask]";
    return s.str();
}