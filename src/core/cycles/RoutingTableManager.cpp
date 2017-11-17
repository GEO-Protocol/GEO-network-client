#include "RoutingTableManager.h"

RoutingTableManager::RoutingTableManager(
    as::io_service &ioService,
    Logger &logger):
    mIOService(ioService),
    mLog(logger)
{
    int timeStarted = 80 + rand() % (60);
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

void RoutingTableManager::updateMapAddOneNeighbor(
    const NodeUUID &firstLevelContractor,
    const NodeUUID &secondLevelContractor)
{
    mRoughtingTable[firstLevelContractor].insert(secondLevelContractor);
}

void RoutingTableManager::updateMapAddSeveralNeighbors(
    const NodeUUID &firstLevelContractor,
    set<NodeUUID> secondLevelContractors)
{
    mLog.debug("updateMapAddSeveralNeighbors") << "Neighbor: " << firstLevelContractor
               << " 2nd level nodes cnt: " << secondLevelContractors.size();
    mRoughtingTable[firstLevelContractor] = secondLevelContractors;
}

set<NodeUUID> RoutingTableManager::secondLevelContractorsForNode(
    const NodeUUID &contractorUUID)
{
    return mRoughtingTable[contractorUUID];
}

void RoutingTableManager::clearMap() {
    mRoughtingTable.clear();
}

void RoutingTableManager::runSignalUpdateTimer(
    const boost::system::error_code &err)
{
    if (err) {
        mLog.error("RoutingTableManager") << err.message();
    }
    mLog.info("runSignalUpdateTimer");
    mUpdatingTimer->cancel();
    mUpdatingTimer->expires_from_now(
        std::chrono::seconds(
            kUpdatingTimerPeriodSeconds));
    mUpdatingTimer->async_wait(
        boost::bind(
            &RoutingTableManager::runSignalUpdateTimer,
            this,
            as::placeholders::error));
    updateRoutingTableSignal();
}
