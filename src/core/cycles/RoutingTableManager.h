#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLESBLUERAYEDITION_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLESBLUERAYEDITION_H

#include "../common/NodeUUID.h"
#include "../logger/Logger.h"
#include "../common/Types.h"

#include <map>
#include <set>

class RoutingTableManager {

public:
    RoutingTableManager(
        const SerializedEquivalent equivalent,
        Logger &logger);

    void updateMapAddSeveralNeighbors(
        const NodeUUID &firstLevelContractor,
        set<NodeUUID> secondLevelContractors);

    void clearMap();

    set<NodeUUID> secondLevelContractorsForNode(
        const NodeUUID &contractorUUID);

protected:
    string logHeader() const;

    LoggerStream warning() const;

    LoggerStream error() const;

    LoggerStream info() const;

    LoggerStream debug() const;

protected:
    SerializedEquivalent mEquivalent;
    map<NodeUUID, set<NodeUUID>> mRoutingTable;
    Logger &mLog;
};

#endif //GEO_NETWORK_CLIENT_ROUTINGTABLESBLUERAYEDITION_H
