#include "EquivalentsSubsystemsRouter.h"

EquivalentsSubsystemsRouter::EquivalentsSubsystemsRouter(
    StorageHandler *storageHandler,
    Keystore *keystore,
    ContractorsManager *contractorsManager,
    EventsInterface *eventsInterface,
    as::io_service &ioService,
    vector<SerializedEquivalent> &equivalentsIAmGateway,
    Logger &logger):

    mStorageHandler(storageHandler),
    mKeysStore(keystore),
    mContractorsManager(contractorsManager),
    mEventsInterface(eventsInterface),
    mIOService(ioService),
    mLogger(logger)
{
    {
        auto ioTransaction = storageHandler->beginTransaction();
        mEquivalents = ioTransaction->trustLinesHandler()->equivalents();
    }
    for (const auto &equivalent : mEquivalents) {
        info() << "Equivalent " << equivalent;

        mIAmGateways.insert(
            make_pair(
                equivalent,
                find(
                    equivalentsIAmGateway.begin(),
                    equivalentsIAmGateway.end(),
                    equivalent) != equivalentsIAmGateway.end()));

        mTrustLinesManagers.insert(
            make_pair(
                equivalent,
                make_unique<TrustLinesManager>(
                    equivalent,
                    mStorageHandler,
                    mKeysStore,
                    mContractorsManager,
                    mLogger)));
        info() << "Trust Lines Manager is successfully initialized";

        mTopologyTrustLinesManagers.insert(
            make_pair(
                equivalent,
                make_unique<TopologyTrustLinesManager>(
                    equivalent,
                    mIAmGateways[equivalent],
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
                    mTrustLinesManagers[equivalent].get(),
                    mTopologyTrustLinesManagers[equivalent].get(),
                    mLogger)));
        info() << "Paths Manager is successfully initialized";
    }

    for (const auto &trustLinesManager : mTrustLinesManagers) {
        for (const auto &contractorID : trustLinesManager.second->contractorsShouldBePinged()) {
            mContractorsShouldBePinged.insert(contractorID);
        }
        trustLinesManager.second->clearContractorsShouldBePinged();
    }

    mGatewayNotificationAndRoutingTablesDelayedTask = make_unique<GatewayNotificationAndRoutingTablesDelayedTask>(
        mIOService,
        mLogger);
    subscribeForGatewayNotification(
        mGatewayNotificationAndRoutingTablesDelayedTask->gatewayNotificationSignal);
    info() << "Gateway Notification and Routing Tables Delayed Task is successfully initialized";
}

vector<SerializedEquivalent> EquivalentsSubsystemsRouter::equivalents() const
{
    return mEquivalents;
}

bool EquivalentsSubsystemsRouter::iAmGateway(
    const SerializedEquivalent equivalent) const
{
    if (mIAmGateways.count(equivalent) == 0) {
        throw NotFoundError(
                "EquivalentsSubsystemsRouter::iAmGateway: "
                    "wrong equivalent " + to_string(equivalent));
    }
    return mIAmGateways.at(equivalent);
}

TrustLinesManager* EquivalentsSubsystemsRouter::trustLinesManager(
    const SerializedEquivalent equivalent) const
{
    if (mTrustLinesManagers.count(equivalent) == 0) {
        throw NotFoundError(
                "EquivalentsSubsystemsRouter::trustLinesManager: "
                    "wrong equivalent " + to_string(equivalent));
    }
    return mTrustLinesManagers.at(equivalent).get();
}

TopologyTrustLinesManager* EquivalentsSubsystemsRouter::topologyTrustLineManager(
    const SerializedEquivalent equivalent) const
{
    if (mTopologyTrustLinesManagers.count(equivalent) == 0) {
        throw NotFoundError(
                "EquivalentsSubsystemsRouter::topologyTrustLineManager: "
                    "wrong equivalent " + to_string(equivalent));
    }
    return mTopologyTrustLinesManagers.at(equivalent).get();
}

TopologyCacheManager* EquivalentsSubsystemsRouter::topologyCacheManager(
    const SerializedEquivalent equivalent) const
{
    if (mTopologyCacheManagers.count(equivalent) == 0) {
        throw NotFoundError(
                "EquivalentsSubsystemsRouter::topologyCacheManager: "
                    "wrong equivalent " + to_string(equivalent));
    }
    return mTopologyCacheManagers.at(equivalent).get();
}

MaxFlowCacheManager* EquivalentsSubsystemsRouter::maxFlowCacheManager(
    const SerializedEquivalent equivalent) const
{
    if (mMaxFlowCacheManagers.count(equivalent) == 0) {
        throw NotFoundError(
                "EquivalentsSubsystemsRouter::maxFlowCacheManager: "
                    "wrong equivalent " + to_string(equivalent));
    }
    return mMaxFlowCacheManagers.at(equivalent).get();
}

PathsManager* EquivalentsSubsystemsRouter::pathsManager(
    const SerializedEquivalent equivalent) const
{
    if (mPathsManagers.count(equivalent) == 0) {
        throw NotFoundError(
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
                "EquivalentsSubsystemsRouter::initNewEquivalent: "
                    "try init equivalent " + to_string(equivalent) + " which is already exists");
    }

    mIAmGateways.insert(
        make_pair(
            equivalent,
            false));

    mTrustLinesManagers.insert(
        make_pair(
            equivalent,
            make_unique<TrustLinesManager>(
                equivalent,
                mStorageHandler,
                mKeysStore,
                mContractorsManager,
                mLogger)));
    info() << "Trust Lines Manager is successfully initialized";

    mTopologyTrustLinesManagers.insert(
        make_pair(
            equivalent,
            make_unique<TopologyTrustLinesManager>(
                equivalent,
                false,
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
                 mTrustLinesManagers[equivalent].get(),
                 mTopologyTrustLinesManagers[equivalent].get(),
                 mLogger)));
    info() << "Paths Manager is successfully initialized";

    mEquivalents.push_back(equivalent);
}

set<ContractorID> EquivalentsSubsystemsRouter::contractorsShouldBePinged() const
{
    return mContractorsShouldBePinged;
}

void EquivalentsSubsystemsRouter::clearContractorsShouldBePinged()
{
    mContractorsShouldBePinged.clear();
}

void EquivalentsSubsystemsRouter::sendTopologyEvent() const
{
    try {
        mEventsInterface->writeEvent(
            Event::topologyEvent(
                mContractorsManager->selfContractor()->mainAddress()));
    } catch (std::exception &e) {
        warning() << "Can't write topology event " << e.what();
    }
}

void EquivalentsSubsystemsRouter::subscribeForGatewayNotification(
    GatewayNotificationAndRoutingTablesDelayedTask::GatewayNotificationSignal &signal)
{
    signal.connect(
        boost::bind(
            &EquivalentsSubsystemsRouter::onGatewayNotificationSlot,
            this));
}

void EquivalentsSubsystemsRouter::onGatewayNotificationSlot()
{
    gatewayNotificationSignal();
}

#ifdef TESTS
void EquivalentsSubsystemsRouter::setMeAsGateway()
{
    for (auto iAmGateway : mIAmGateways) {
        iAmGateway.second = true;
    }
}
#endif

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
