#include "EquivalentsSubsystemsRouter.h"

EquivalentsSubsystemsRouter::EquivalentsSubsystemsRouter(
    NodeUUID &nodeUUID,
    StorageHandler *storageHandler,
    as::io_service &ioService,
    bool iAmGateway,
    Logger &logger):

    mNodeUUID(nodeUUID),
    mIAmGateway(iAmGateway),
    mStorageHandler(storageHandler),
    mIOService(ioService),
    mLogger(logger)
{
    // todo: uncomment me after applying new equivalent adding logic
//    {
//        auto ioTransaction = storageHandler->beginTransaction();
//        mEquivalents = ioTransaction->trustLinesHandler()->equivalents();
//    }
    mEquivalents.push_back(1);
    for (const auto &equivalent : mEquivalents) {
        info() << "Equivalent " << equivalent;
        mTrustLinesManagers.insert(
            make_pair(
                equivalent,
                make_unique<TrustLinesManager>(
                    equivalent,
                    mStorageHandler,
                    mLogger)));
        info() << "Trust Lines Manager is successfully initialized";

        mTopologyTrustLinesManagers.insert(
            make_pair(
                equivalent,
                make_unique<TopologyTrustLinesManager>(
                    equivalent,
                    mIAmGateway,
                    mNodeUUID,
                    mLogger)));
        info() << "Topology Trust Lines Manager is successfully initialized";

        mTopologyCacheManagers.insert(
            make_pair(
                equivalent,
                make_unique<TopologyCacheManager>(
                    equivalent,
                    mLogger)));
        info() << "Topology Cache Manager is successfully initialized";

        mMaxFlowCacheManagers.insert(
            make_pair(
                equivalent,
                make_unique<MaxFlowCacheManager>(
                    equivalent,
                    mLogger)));
        info() << "Max Flow Cache Manager is successfully initialized";

        mTopologyCacheUpdateDelayedTasks.insert(
            make_pair(
                equivalent,
                make_unique<TopologyCacheUpdateDelayedTask>(
                    equivalent,
                    mIOService,
                    mTopologyCacheManagers[equivalent].get(),
                    mTopologyTrustLinesManagers[equivalent].get(),
                    mMaxFlowCacheManagers[equivalent].get(),
                    mLogger)));
        info() << "Topology Cache Update Delayed Task is successfully initialized";

        mPathsManagers.insert(
            make_pair(
                equivalent,
                make_unique<PathsManager>(
                    equivalent,
                    mNodeUUID,
                    mTrustLinesManagers[equivalent].get(),
                    mTopologyTrustLinesManagers[equivalent].get(),
                    mLogger)));
        info() << "Paths Manager is successfully initialized";

        mNotifyThatIAmIsGatewayDelayedTasks.insert(
            make_pair(
                equivalent,
                make_unique<NotifyThatIAmIsGatewayDelayedTask>(
                    equivalent,
                    mIOService,
                    mLogger)));
        info() << "Gateway Notification Delayed Task is successfully initialized";
    }

    connectSignalsToSlots();
}

vector<SerializedEquivalent> EquivalentsSubsystemsRouter::equivalents() const
{
    return mEquivalents;
}

TrustLinesManager* EquivalentsSubsystemsRouter::trustLinesManager(
    const SerializedEquivalent equivalent) const
{
    if (mTrustLinesManagers.count(equivalent) == 0) {
        throw ValueError(
                "EquivalentsSubsystemsRouter::routingTableManager: "
                    "wrong equivalent " + to_string(equivalent));
    }
    return mTrustLinesManagers.at(equivalent).get();
}

TopologyTrustLinesManager* EquivalentsSubsystemsRouter::topologyTrustLineManager(
    const SerializedEquivalent equivalent) const
{
    if (mTopologyTrustLinesManagers.count(equivalent) == 0) {
        throw ValueError(
                "EquivalentsSubsystemsRouter::topologyTrustLineManager: "
                    "wrong equivalent " + to_string(equivalent));
    }
    return mTopologyTrustLinesManagers.at(equivalent).get();
}

TopologyCacheManager* EquivalentsSubsystemsRouter::topologyCacheManager(
    const SerializedEquivalent equivalent) const
{
    if (mTopologyCacheManagers.count(equivalent) == 0) {
        throw ValueError(
                "EquivalentsSubsystemsRouter::topologyCacheManager: "
                    "wrong equivalent " + to_string(equivalent));
    }
    return mTopologyCacheManagers.at(equivalent).get();
}

MaxFlowCacheManager* EquivalentsSubsystemsRouter::maxFlowCacheManager(
    const SerializedEquivalent equivalent) const
{
    if (mMaxFlowCacheManagers.count(equivalent) == 0) {
        throw ValueError(
                "EquivalentsSubsystemsRouter::maxFlowCacheManager: "
                    "wrong equivalent " + to_string(equivalent));
    }
    return mMaxFlowCacheManagers.at(equivalent).get();
}

PathsManager* EquivalentsSubsystemsRouter::pathsManager(
    const SerializedEquivalent equivalent) const
{
    if (mPathsManagers.count(equivalent) == 0) {
        throw ValueError(
                "EquivalentsSubsystemsRouter::pathsManager: "
                    "wrong equivalent " + to_string(equivalent));
    }
    return mPathsManagers.at(equivalent).get();
}

void EquivalentsSubsystemsRouter::initNewEquivalent(
    const SerializedEquivalent equivalent)
{

    if (mTrustLinesManagers.count(equivalent) != 0) {
        throw ValueError(
                "EquivalentsCyclesSubsystemsRouter::initNewEquivalent: "
                    "try init equivalent which is already exists");
    }

    mTrustLinesManagers.insert(
        make_pair(
            equivalent,
            make_unique<TrustLinesManager>(
                equivalent,
                mStorageHandler,
                mLogger)));
    info() << "Trust Lines Manager is successfully initialized";

    mTopologyTrustLinesManagers.insert(
        make_pair(
            equivalent,
            make_unique<TopologyTrustLinesManager>(
                equivalent,
                mIAmGateway,
                mNodeUUID,
                mLogger)));
    info() << "Topology Trust Lines Manager is successfully initialized";

    mTopologyCacheManagers.insert(
        make_pair(
            equivalent,
            make_unique<TopologyCacheManager>(
                equivalent,
                mLogger)));
    info() << "Topology Cache Manager is successfully initialized";

    mMaxFlowCacheManagers.insert(
        make_pair(
            equivalent,
            make_unique<MaxFlowCacheManager>(
                equivalent,
                mLogger)));
    info() << "Max Flow Cache Manager is successfully initialized";

    mTopologyCacheUpdateDelayedTasks.insert(
        make_pair(
            equivalent,
            make_unique<TopologyCacheUpdateDelayedTask>(
                equivalent,
                mIOService,
                mTopologyCacheManagers[equivalent].get(),
                mTopologyTrustLinesManagers[equivalent].get(),
                mMaxFlowCacheManagers[equivalent].get(),
                mLogger)));
    info() << "Topology Cache Update Delayed Task is successfully initialized";

    mPathsManagers.insert(
        make_pair(
             equivalent,
             make_unique<PathsManager>(
                 equivalent,
                 mNodeUUID,
                 mTrustLinesManagers[equivalent].get(),
                 mTopologyTrustLinesManagers[equivalent].get(),
                 mLogger)));
    info() << "Paths Manager is successfully initialized";

    mNotifyThatIAmIsGatewayDelayedTasks.insert(
        make_pair(
           equivalent,
           make_unique<NotifyThatIAmIsGatewayDelayedTask>(
               equivalent,
               mIOService,
               mLogger)));
    subscribeForGatewayNotification(
        mNotifyThatIAmIsGatewayDelayedTasks[equivalent]->gatewayNotificationSignal);
    info() << "Gateway Notification Delayed Task is successfully initialized";
}

void EquivalentsSubsystemsRouter::connectSignalsToSlots()
{
    for (const auto &notifyThatIAmIsGatewayDelayedTask : mNotifyThatIAmIsGatewayDelayedTasks) {
        subscribeForGatewayNotification(
            notifyThatIAmIsGatewayDelayedTask.second->gatewayNotificationSignal);
    }
}

void EquivalentsSubsystemsRouter::subscribeForGatewayNotification(
    NotifyThatIAmIsGatewayDelayedTask::GatewayNotificationSignal &signal)
{
    signal.connect(
        boost::bind(
            &EquivalentsSubsystemsRouter::onGatewayNotificationSlot,
            this,
            _1));
}

void EquivalentsSubsystemsRouter::onGatewayNotificationSlot(
    const SerializedEquivalent equivalent)
{
    gatewayNotificationSignal(equivalent);
}

string EquivalentsSubsystemsRouter::logHeader() const
{
    return "[EquivalentsSubsystemsRouter]";
}

LoggerStream EquivalentsSubsystemsRouter::error() const
{
    return mLogger.error(logHeader());
}

LoggerStream EquivalentsSubsystemsRouter::warning() const
{
    return mLogger.warning(logHeader());
}

LoggerStream EquivalentsSubsystemsRouter::info() const
{
    return mLogger.info(logHeader());
}

LoggerStream EquivalentsSubsystemsRouter::debug() const
{
    return mLogger.debug(logHeader());
}
