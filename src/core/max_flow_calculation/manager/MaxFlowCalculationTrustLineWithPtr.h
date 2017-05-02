#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINEWITHPTR_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINEWITHPTR_H

#include "../MaxFlowCalculationTrustLine.h"
#include <unordered_set>

class MaxFlowCalculationTrustLineWithPtr {

public:
    MaxFlowCalculationTrustLineWithPtr(
        const MaxFlowCalculationTrustLine::Shared maxFlowCalculationTrustLine,
        unordered_set<MaxFlowCalculationTrustLineWithPtr*>* hashMapPtr);

    MaxFlowCalculationTrustLine::Shared maxFlowCalculationtrustLine();

    unordered_set<MaxFlowCalculationTrustLineWithPtr*>* hashSetPtr();

private:
    MaxFlowCalculationTrustLine::Shared mMaxFlowCalulationTrustLine;
    unordered_set<MaxFlowCalculationTrustLineWithPtr*>* mHashSetPtr;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINEWITHPTR_H
