#ifndef GEO_NETWORK_CLIENT_TOPOLOGYTRUSTLINESMANAGER_H
#define GEO_NETWORK_CLIENT_TOPOLOGYTRUSTLINESMANAGER_H

#include "../../common/NodeUUID.h"
#include "TopologyTrustLineWithPtr.h"
#include "../../common/time/TimeUtils.h"
#include "../../logger/Logger.h"

#include <set>
#include <unordered_set>
#include <unordered_map>
#include <boost/functional/hash.hpp>

class TopologyTrustLinesManager {

public:
    typedef unordered_set<TopologyTrustLineWithPtr*> TrustLineWithPtrHashSet;

public:
    TopologyTrustLinesManager(
        const SerializedEquivalent equivalent,
        bool iAmGateway,
        NodeUUID &nodeUUID,
        Logger &logger);

    void addTrustLine(
        TopologyTrustLine::Shared trustLine);

    TrustLineWithPtrHashSet trustLinePtrsSet(
        const NodeUUID &nodeUUID);

    void resetAllUsedAmounts();

    bool deleteLegacyTrustLines();

    size_t trustLinesCounts() const;

    // todo : this code used only for testing and should be deleted in future
    void printTrustLines() const;

    DateTime closestTimeEvent() const;

    void setPreventDeleting(
        bool preventDeleting);

    bool preventDeleting() const;

    void addUsedAmount(
        const NodeUUID &sourceUUID,
        const NodeUUID &targetUUID,
        const TrustLineAmount &amount);

    void makeFullyUsed(
        const NodeUUID &sourceUUID,
        const NodeUUID &targetUUID);

    set<NodeUUID> neighborsOf(
        const NodeUUID &sourceUUID);

    void addGateway(const NodeUUID &gateway);

    const set<NodeUUID> gateways() const;

    void makeFullyUsedTLsFromGatewaysToAllNodesExceptOne(
        const NodeUUID &exceptedNode);

private:
    static const byte kResetTrustLinesHours = 0;
    static const byte kResetTrustLinesMinutes = 12;
    static const byte kResetTrustLinesSeconds = 0;

    static Duration& kResetTrustLinesDuration() {
        static auto duration = Duration(
            kResetTrustLinesHours,
            kResetTrustLinesMinutes,
            kResetTrustLinesSeconds);
        return duration;
    }

    static const byte kClearTrustLinesHours = 0;
    static const byte kClearTrustLinesMinutes = 30;
    static const byte kClearTrustLinesSeconds = 0;

    static Duration& kClearTrustLinesDuration() {
        static auto duration = Duration(
            kClearTrustLinesHours,
            kClearTrustLinesMinutes,
            kClearTrustLinesSeconds);
        return duration;
    }

private:
    LoggerStream info() const;
    LoggerStream debug() const;

    const string logHeader() const;

private:
    unordered_map<NodeUUID, TrustLineWithPtrHashSet*, boost::hash<boost::uuids::uuid>> msTrustLines;
    map<DateTime, TopologyTrustLineWithPtr*> mtTrustLines;
    SerializedEquivalent mEquivalent;
    Logger &mLog;
    bool mPreventDeleting;
    set<NodeUUID> mGateways;
    DateTime mLastTrustLineTimeAdding;
};

#endif //GEO_NETWORK_CLIENT_TOPOLOGYTRUSTLINESMANAGER_H
