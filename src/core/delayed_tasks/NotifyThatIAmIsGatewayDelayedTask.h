#ifndef GEO_NETWORK_CLIENT_NOTIFYTHATIAMISGATEWAYDELAYEDTASK_H
#define GEO_NETWORK_CLIENT_NOTIFYTHATIAMISGATEWAYDELAYEDTASK_H

#include "../common/time/TimeUtils.h"
#include "../logger/Logger.h"
#include "../common/Types.h"

#include <boost/asio/steady_timer.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/signals2.hpp>
#include <boost/asio.hpp>

using namespace std;
namespace as = boost::asio;
namespace signals = boost::signals2;

class NotifyThatIAmIsGatewayDelayedTask {

public:
    typedef signals::signal<void()> GatewayNotificationSignal;

public:
    NotifyThatIAmIsGatewayDelayedTask(
        as::io_service &mIOService,
        Logger &logger);

public:
    mutable GatewayNotificationSignal gatewayNotificationSignal;

private:
    void runSignalNotify(
        const boost::system::error_code &error);

    LoggerStream info() const;

    LoggerStream debug() const;

    LoggerStream warning() const;

    const string logHeader() const;

private:
    static const uint32_t kUpdatingTimerPeriodSeconds = 60 * 60 * 24 * 3;

private:
    as::io_service &mIOService;
    unique_ptr<as::steady_timer> mNotificationTimer;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_NOTIFYTHATIAMISGATEWAYDELAYEDTASK_H
