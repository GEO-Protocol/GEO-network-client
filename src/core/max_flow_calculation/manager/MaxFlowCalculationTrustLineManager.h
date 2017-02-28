#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINEMANAGER_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINEMANAGER_H

#include "../../common/NodeUUID.h"
#include "../MaxFlowCalculationTrustLine.h"

#include <map>
#include <vector>

#include "boost/container/flat_set.hpp"

//using namespace std;
class MaxFlowCalculationTrustLineManager {

public:

    void addTrustLine(MaxFlowCalculationTrustLine::Shared trustLine);

    vector<MaxFlowCalculationTrustLine::Shared> sortedTrustLines(const NodeUUID &nodeUUID);

    void resetAllUsedAmounts();

private:
    // sort using a custom function object
    struct {
        bool operator()(
            MaxFlowCalculationTrustLine::Shared a,
            MaxFlowCalculationTrustLine::Shared b) {
            auto aTrustLineFreeAmountPtr = a.get()->freeAmount();
            auto bTrustLineFreeAmountPtr = b.get()->freeAmount();
            return *aTrustLineFreeAmountPtr > *bTrustLineFreeAmountPtr;
        }
    } customLess;

// todo make private after testing
public:
    map<NodeUUID, vector<MaxFlowCalculationTrustLine::Shared>> mvTrustLines;

};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINEMANAGER_H
