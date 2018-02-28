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
                    mIOService,
                    mLogger)));
        info() << "Routing Table Manager is successfully initialized";
    }

    connectSignalsToSlots();
}

CyclesManager* EquivalentsCyclesSubsystemsRouter::cyclesManager(
    const SerializedEquivalent equivalent) const
{
    return mCyclesManagers.at(equivalent).get();
}

RoutingTableManager* EquivalentsCyclesSubsystemsRouter::routingTableManager(
    const SerializedEquivalent equivalent) const
{
    return mRoutingTablesManagers.at(equivalent).get();
}

void EquivalentsCyclesSubsystemsRouter::connectSignalsToSlots()
{
    for (const auto &cyclesManager : mCyclesManagers) {
        cyclesManager.second->buildFiveNodesCyclesSignal.connect(
            boost::bind(
                &EquivalentsCyclesSubsystemsRouter::onBuildCycleFiveNodesSlot,
                this,
                _1));

        cyclesManager.second->buildSixNodesCyclesSignal.connect(
            boost::bind(
                &EquivalentsCyclesSubsystemsRouter::onBuildCycleSixNodesSlot,
                this,
                _1));

        cyclesManager.second->closeCycleSignal.connect(
            boost::bind(
                &EquivalentsCyclesSubsystemsRouter::onCloseCycleSlot,
                this,
                _1,
                _2));
    }

    for (const auto &routingTablesManager : mRoutingTablesManagers) {
        routingTablesManager.second->updateRoutingTableSignal.connect(
            boost::bind(
                &EquivalentsCyclesSubsystemsRouter::onUpdateRoutingTableSlot,
                this,
                _1));
    }
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

void EquivalentsCyclesSubsystemsRouter::onUpdateRoutingTableSlot(
    const SerializedEquivalent equivalent)
{
    updateRoutingTableSignal(equivalent);
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