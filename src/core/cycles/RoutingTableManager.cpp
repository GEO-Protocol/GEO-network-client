#include "RoutingTableManager.h"

RoutingTableManager::RoutingTableManager(
    const SerializedEquivalent equivalent,
    Logger &logger):

    mEquivalent(equivalent),
    mLog(logger)
{}

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

void RoutingTableManager::clearMap()
{
    mRoutingTable.clear();
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
