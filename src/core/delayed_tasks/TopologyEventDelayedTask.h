#ifndef GEO_NETWORK_CLIENT_TOPOLOGYEVENTDELAYEDTASK_H
#define GEO_NETWORK_CLIENT_TOPOLOGYEVENTDELAYEDTASK_H

#include "../equivalents/EquivalentsSubsystemsRouter.h"

#include <boost/asio/steady_timer.hpp>
#include <boost/asio.hpp>

class TopologyEventDelayedTask {

public:
    TopologyEventDelayedTask(
        as::io_service &ioService,
        EquivalentsSubsystemsRouter *equivalentsSubsystemsRouter,
        Logger &logger);

private:
    void runTopologyEvent();

private:
    static const uint16_t kDelayedTaskTimeSec = 5;

private:
    as::io_service &mIOService;
    unique_ptr<as::steady_timer> mTopologyEventTimer;
    EquivalentsSubsystemsRouter *mEquivalentsSubsystemsRouter;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_TOPOLOGYEVENTDELAYEDTASK_H
