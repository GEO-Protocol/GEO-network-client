#include "TopologyEventDelayedTask.h"

TopologyEventDelayedTask::TopologyEventDelayedTask(
    as::io_service &ioService,
    EquivalentsSubsystemsRouter *equivalentsSubsystemsRouter,
    Logger &logger) :
    mIOService(ioService),
    mEquivalentsSubsystemsRouter(equivalentsSubsystemsRouter),
    mLog(logger)
{
    mTopologyEventTimer = make_unique<as::steady_timer>(
        mIOService);

    mTopologyEventTimer->expires_from_now(
        chrono::seconds(
            +kDelayedTaskTimeSec));
    mTopologyEventTimer->async_wait(boost::bind(
        &TopologyEventDelayedTask::runTopologyEvent,
        this));
}

void TopologyEventDelayedTask::runTopologyEvent()
{
    mTopologyEventTimer->cancel();
    mEquivalentsSubsystemsRouter->sendTopologyEvent();
}