#include "RoutingTableManager.h"

RoutingTableManager::RoutingTableManager(
    const SerializedEquivalent equivalent,
    Logger &logger):

    mEquivalent(equivalent),
    mLog(logger)
{}

void RoutingTableManager::updateMapAddSeveralNeighbors(
    ContractorID firstLevelContractor,
    vector<BaseAddress::Shared> secondLevelContractors)
{
    debug() << "updateMapAddSeveralNeighbors. Neighbor: " << firstLevelContractor
            << " 2nd level nodes cnt: " << secondLevelContractors.size();
    mRoutingTable[firstLevelContractor] = secondLevelContractors;
}

vector<BaseAddress::Shared> RoutingTableManager::secondLevelContractorsForNode(
    ContractorID contractorID)
{
    return mRoutingTable[contractorID];
}

void RoutingTableManager::clearMap()
{
    mRoutingTable.clear();
}

void RoutingTableManager::printRT()
{
    debug() << "Print RTs";
    for (const auto &contractorAndNeighbors : mRoutingTable) {
        debug() << "Contractor " << contractorAndNeighbors.first;
        debug() << "\tNeighbors:";
        for (const auto &neighbor : contractorAndNeighbors.second) {
            debug() << "\t" << neighbor->fullAddress();
        }
    }
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
