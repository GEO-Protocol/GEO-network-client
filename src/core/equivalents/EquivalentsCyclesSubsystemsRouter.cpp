#include "EquivalentsCyclesSubsystemsRouter.h"

EquivalentsCyclesSubsystemsRouter::EquivalentsCyclesSubsystemsRouter(
    NodeUUID &nodeUUID,
    TransactionsScheduler *transactionScheduler,
    SubsystemsController *subsystemsController,
    as::io_service &ioService,
    vector<SerializedEquivalent> equivalents,
    Logger &logger):

    mNodeUUID(nodeUUID),
    mTransactionScheduler(transactionScheduler),
    mSubsystemsController(subsystemsController),
    mIOService(ioService),
    mLogger(logger)
{
    for (const auto &equivalent : equivalents) {
        info() << "Equivalent " << equivalent;
        mCyclesManagers.insert(
            make_pair(
                equivalent,
                make_unique<CyclesManager>(
                    equivalent,
                    mNodeUUID,
                    mTransactionScheduler,
                    mIOService,
                    mLogger,
                    mSubsystemsController)));
        info() << "Cycles Manager is successfully initialized";

        mRoutingTablesManagers.insert(
            make_pair(
                equivalent,
                make_unique<RoutingTableManager>(
                    equivalent,
                    mLogger)));
        info() << "Routing Table Manager is successfully initialized";
    }

    connectSignalsToSlots();
}

CyclesManager* EquivalentsCyclesSubsystemsRouter::cyclesManager(
    const SerializedEquivalent equivalent) const
{
    if (mCyclesManagers.count(equivalent) == 0) {
        throw NotFoundError(
                "EquivalentsCyclesSubsystemsRouter::cyclesManager: "
                    "wrong equivalent " + to_string(equivalent));
    }
    return mCyclesManagers.at(equivalent).get();
}

RoutingTableManager* EquivalentsCyclesSubsystemsRouter::routingTableManager(
    const SerializedEquivalent equivalent) const
{
    if (mRoutingTablesManagers.count(equivalent) == 0) {
        throw NotFoundError(
                "EquivalentsCyclesSubsystemsRouter::routingTableManager: "
                    "wrong equivalent " + to_string(equivalent));
    }
    return mRoutingTablesManagers.at(equivalent).get();
}

void EquivalentsCyclesSubsystemsRouter::initNewEquivalent(
    const SerializedEquivalent equivalent)
{
    if (mCyclesManagers.count(equivalent) != 0) {
        throw ValueError(
                "EquivalentsCyclesSubsystemsRouter::initNewEquivalent: "
                    "try init equivalent " + to_string(equivalent) + " which is already exists");
    }

    mCyclesManagers.insert(
        make_pair(
            equivalent,
            make_unique<CyclesManager>(
                equivalent,
                mNodeUUID,
                mTransactionScheduler,
                mIOService,
                mLogger,
                mSubsystemsController)));
    subscribeForBuildingFiveNodesCycles(
        mCyclesManagers[equivalent]->buildFiveNodesCyclesSignal);
    subscribeForBuildingSixNodesCycles(
        mCyclesManagers[equivalent]->buildSixNodesCyclesSignal);
    subscribeForClosingCycles(
        mCyclesManagers[equivalent]->closeCycleSignal);
    info() << "Cycles Manager is successfully initialized";

    mRoutingTablesManagers.insert(
        make_pair(
            equivalent,
            make_unique<RoutingTableManager>(
                equivalent,
                mLogger)));
    info() << "Routing Table Manager is successfully initialized";
}

void EquivalentsCyclesSubsystemsRouter::clearRoutingTables()
{
    info() << "clearRoutingTables";
    for (const auto &routingTablesManager : mRoutingTablesManagers) {
        info() << "clearRoutingTables for " << routingTablesManager.first;
        routingTablesManager.second->clearMap();
    }
}

void EquivalentsCyclesSubsystemsRouter::connectSignalsToSlots()
{
    for (const auto &cyclesManager : mCyclesManagers) {
        subscribeForBuildingFiveNodesCycles(
            cyclesManager.second->buildFiveNodesCyclesSignal);
        subscribeForBuildingSixNodesCycles(
            cyclesManager.second->buildSixNodesCyclesSignal);
        subscribeForClosingCycles(
            cyclesManager.second->closeCycleSignal);
    }
}

void EquivalentsCyclesSubsystemsRouter::subscribeForBuildingFiveNodesCycles(
    CyclesManager::BuildFiveNodesCyclesSignal &signal)
{
    signal.connect(
        boost::bind(
            &EquivalentsCyclesSubsystemsRouter::onBuildCycleFiveNodesSlot,
            this,
            _1));
}

void EquivalentsCyclesSubsystemsRouter::subscribeForBuildingSixNodesCycles(
    CyclesManager::BuildSixNodesCyclesSignal &signal)
{
    signal.connect(
        boost::bind(
            &EquivalentsCyclesSubsystemsRouter::onBuildCycleSixNodesSlot,
            this,
            _1));
}

void EquivalentsCyclesSubsystemsRouter::subscribeForClosingCycles(
    CyclesManager::CloseCycleSignal &signal)
{
    signal.connect(
        boost::bind(
            &EquivalentsCyclesSubsystemsRouter::onCloseCycleSlot,
            this,
            _1,
            _2));
}

void EquivalentsCyclesSubsystemsRouter::onBuildCycleFiveNodesSlot(
    const SerializedEquivalent equivalent)
{
    buildFiveNodesCyclesSignal(equivalent);
}

void EquivalentsCyclesSubsystemsRouter::onBuildCycleSixNodesSlot(
    const SerializedEquivalent equivalent)
{
    buildSixNodesCyclesSignal(equivalent);
}

void EquivalentsCyclesSubsystemsRouter::onCloseCycleSlot(
    const SerializedEquivalent equivalent,
    Path::ConstShared cycle)
{
    closeCycleSignal(
        equivalent,
        cycle);
}

string EquivalentsCyclesSubsystemsRouter::logHeader() const
{
    return "[EquivalentsCyclesSubsystemsRouter]";
}

LoggerStream EquivalentsCyclesSubsystemsRouter::error() const
{
    return mLogger.error(logHeader());
}

LoggerStream EquivalentsCyclesSubsystemsRouter::warning() const
{
    return mLogger.warning(logHeader());
}

LoggerStream EquivalentsCyclesSubsystemsRouter::info() const
{
    return mLogger.info(logHeader());
}

LoggerStream EquivalentsCyclesSubsystemsRouter::debug() const
{
    return mLogger.debug(logHeader());
}