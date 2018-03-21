#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLESBLUERAYEDITION_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLESBLUERAYEDITION_H

#include "../common/NodeUUID.h"
#include "../logger/Logger.h"
#include "../common/Types.h"

#include <boost/signals2.hpp>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

#include <map>
#include <set>

namespace as = boost::asio;
namespace signals = boost::signals2;


class RoutingTableManager {
public:
    typedef signals::signal<void(const SerializedEquivalent equivalent)> UpdateRoutingTableSignal;

public:
    RoutingTableManager(
        const SerializedEquivalent equivalent,
        as::io_service &ioService,
        Logger &logger);

    void updateMapAddSeveralNeighbors(
        const NodeUUID &firstLevelContractor,
        set<NodeUUID> secondLevelContractors);

    void clearMap();

    set<NodeUUID> secondLevelContractorsForNode(
        const NodeUUID &contractorUUID);

    void runSignalUpdateTimer(
        const boost::system::error_code &err);

public:
    mutable UpdateRoutingTableSignal updateRoutingTableSignal;

protected:
    string logHeader() const;

    LoggerStream warning() const;

    LoggerStream error() const;

    LoggerStream info() const;

    LoggerStream debug() const;

protected:
    const uint32_t kUpdatingTimerPeriodSeconds = 60 * 60 * 24 * 3;

protected:
    SerializedEquivalent mEquivalent;
    as::io_service &mIOService;
    map<NodeUUID, set<NodeUUID>> mRoutingTable;
    Logger &mLog;

    unique_ptr<as::steady_timer> mUpdatingTimer;
};

#endif //GEO_NETWORK_CLIENT_ROUTINGTABLESBLUERAYEDITION_H
