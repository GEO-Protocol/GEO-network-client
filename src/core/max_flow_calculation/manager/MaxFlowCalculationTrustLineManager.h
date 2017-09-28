#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINEMANAGER_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINEMANAGER_H

#include "../../common/NodeUUID.h"
#include "MaxFlowCalculationTrustLineWithPtr.h"
#include "../../cycles/RoutingTableManager.h"
#include "../../common/time/TimeUtils.h"
#include "../../logger/Logger.h"

#include <set>
#include <unordered_set>
#include <unordered_map>
#include <boost/functional/hash.hpp>

class MaxFlowCalculationTrustLineManager {

public:
    typedef unordered_set<MaxFlowCalculationTrustLineWithPtr*> TrustLineWithPtrHashSet;

public:
    MaxFlowCalculationTrustLineManager(
        RoutingTableManager *roughtingTable,
        Logger &logger);

    void addTrustLine(
        MaxFlowCalculationTrustLine::Shared trustLine);

    TrustLineWithPtrHashSet trustLinePtrsSet(
        const NodeUUID &nodeUUID);

    void resetAllUsedAmounts();

    bool deleteLegacyTrustLines();

    size_t trustLinesCounts() const;

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

private:
    LoggerStream info() const;
    LoggerStream debug() const;

    const string logHeader() const;

private:
    unordered_map<NodeUUID, TrustLineWithPtrHashSet*, boost::hash<boost::uuids::uuid>> msTrustLines;
    map<DateTime, MaxFlowCalculationTrustLineWithPtr*> mtTrustLines;
    RoutingTableManager *mRoughtingTable;
    Logger &mLog;
    bool mPreventDeleting;
};

#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINEMANAGER_H
