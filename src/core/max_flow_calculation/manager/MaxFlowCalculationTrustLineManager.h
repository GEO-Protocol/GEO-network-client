#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINEMANAGER_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINEMANAGER_H

#include "../../common/NodeUUID.h"
#include "MaxFlowCalculationTrustLineWithPtr.h"
#include "../../common/time/TimeUtils.h"
#include "../../logger/Logger.h"

#include <unordered_map>
#include <unordered_set>

class MaxFlowCalculationTrustLineManager {

public:
    typedef unordered_set<MaxFlowCalculationTrustLineWithPtr*> trustLineWithPtrHashSet;

public:
    MaxFlowCalculationTrustLineManager(Logger *logger);

    void addTrustLine(MaxFlowCalculationTrustLine::Shared trustLine);

    vector<MaxFlowCalculationTrustLine::Shared> sortedTrustLines(const NodeUUID &nodeUUID);

    void resetAllUsedAmounts();

    void deleteLegacyTrustLines();

    size_t trustLinesCounts() const;

    void printTrustLines() const;

    DateTime closestTimeEvent() const;

private:
    // comparing two trustLines for sorting
    struct {
        bool operator()(
            MaxFlowCalculationTrustLine::Shared a,
            MaxFlowCalculationTrustLine::Shared b) {
            auto aTrustLineFreeAmountPtr = a.get()->freeAmount();
            auto bTrustLineFreeAmountPtr = b.get()->freeAmount();
            return *aTrustLineFreeAmountPtr > *bTrustLineFreeAmountPtr;
        }
    } customLess;

private:
    static const byte kResetTrustLinesHours = 0;
    static const byte kResetTrustLinesMinutes = 0;
    static const byte kResetTrustLinesSeconds = 60;

    static Duration& kResetTrustLinesDuration() {
        static auto duration = Duration(
            kResetTrustLinesHours,
            kResetTrustLinesMinutes,
            kResetTrustLinesSeconds);
        return duration;
    }

private:
    LoggerStream info() const;

    const string logHeader() const;

private:
    unordered_map<NodeUUID, trustLineWithPtrHashSet*> msTrustLines;
    map<DateTime, MaxFlowCalculationTrustLineWithPtr*> mtTrustLines;
    Logger *mLog;
};

#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINEMANAGER_H
