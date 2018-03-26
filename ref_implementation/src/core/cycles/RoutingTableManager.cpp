/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "RoutingTableManager.h"

RoutingTableManager::RoutingTableManager(
    as::io_service &ioService,
    Logger &logger):
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
    debug() << "updateMapAddSeveralNeighbors. Neighbor: " << firstLevelContractor
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
    updateRoutingTableSignal();
}

string RoutingTableManager::logHeader()
{
    return "[RoutingTableManager]";
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
