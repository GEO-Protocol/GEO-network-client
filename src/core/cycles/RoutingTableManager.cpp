#include "RoutingTableManager.h"

RoutingTableManager::RoutingTableManager(
    const SerializedEquivalent equivalent,
    as::io_service &ioService,
    Logger &logger):

    mEquivalent(equivalent),
    mIOService(ioService),
    mLog(logger)
{
    int timeStarted = 120 + rand() % (60);
#ifdef TESTS
    timeStarted = 10;
#endif
    mUpdatingTimer = make_unique<as::steady_timer>(
        mIOService);
    mUpdatingTimer->expires_from_now(
        std::chrono::seconds(
            timeStarted));
    mUpdatingTimer->async_wait(
        boost::bind(
            &RoutingTableManager::runSignalUpdateTimer,
            this,
            as::placeholders::error));
}

void RoutingTableManager::updateMapAddSeveralNeighbors(
    const NodeUUID &firstLevelContractor,
    set<NodeUUID> secondLevelContractors)
{
    debug() << "updateMapAddSeveralNeighbors. Neighbor: " << firstLevelContractor
               << " 2nd level nodes cnt: " << secondLevelContractors.size();
    mRoutingTable[firstLevelContractor] = secondLevelContractors;
}

set<NodeUUID> RoutingTableManager::secondLevelContractorsForNode(
    const NodeUUID &contractorUUID)
{
    return mRoutingTable[contractorUUID];
}

void RoutingTableManager::clearMap() {
    mRoutingTable.clear();
}

void RoutingTableManager::runSignalUpdateTimer(
    const boost::system::error_code &err)
{
    if (err) {
        error() << err.message();
    }
    info() << "runSignalUpdateTimer";
    mUpdatingTimer->cancel();
    mUpdatingTimer->expires_from_now(
        std::chrono::seconds(
            kUpdatingTimerPeriodSeconds));
    mUpdatingTimer->async_wait(
        boost::bind(
            &RoutingTableManager::runSignalUpdateTimer,
            this,
            as::placeholders::error));
    updateRoutingTableSignal(mEquivalent);
}

string RoutingTableManager::logHeader() const
{
    stringstream s;
    s << "[RoutingTableManager: " << mEquivalent << "] ";
    return s.str();
}

LoggerStream RoutingTableManager::error() const
{
    return mLog.error(logHeader());
}

LoggerStream RoutingTableManager::warning() const
{
    return mLog.warning(logHeader());
}

LoggerStream RoutingTableManager::info() const
{
    return mLog.info(logHeader());
}

LoggerStream RoutingTableManager::debug() const
{
    return mLog.debug(logHeader());
}
