#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINEMANAGER_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINEMANAGER_H

#include "../../common/NodeUUID.h"
#include "../MaxFlowCalculationTrustLine.h"
#include "../../common/time/TimeUtils.h"

#include <map>
#include <vector>

class MaxFlowCalculationTrustLineManager {

public:

    void addTrustLine(MaxFlowCalculationTrustLine::Shared trustLine);

    vector<MaxFlowCalculationTrustLine::Shared> sortedTrustLines(const NodeUUID &nodeUUID);

    void resetAllUsedAmounts();

    void deleteLegacyTrustLines();

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
    static const byte kResetTrustLinesSeconds = 2;

// todo make private after testing
public:
    map<NodeUUID, vector<MaxFlowCalculationTrustLine::Shared>> mvTrustLines;

};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINEMANAGER_H
