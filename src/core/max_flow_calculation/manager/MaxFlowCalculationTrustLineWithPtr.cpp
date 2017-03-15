#include "MaxFlowCalculationTrustLineWithPtr.h"

MaxFlowCalculationTrustLineWithPtr::MaxFlowCalculationTrustLineWithPtr(
        const MaxFlowCalculationTrustLine::Shared maxFlowCalculationTrustLine,
        unordered_set<MaxFlowCalculationTrustLineWithPtr*>* hashMapPtr) :

        mMaxFlowCalulationTrustLine(maxFlowCalculationTrustLine),
        mHashSetPtr(hashMapPtr) {}

MaxFlowCalculationTrustLine::Shared MaxFlowCalculationTrustLineWithPtr::maxFlowCalculationtrustLine() {
    return mMaxFlowCalulationTrustLine;
}

unordered_set<MaxFlowCalculationTrustLineWithPtr*>* MaxFlowCalculationTrustLineWithPtr::hashSetPtr() {
    return mHashSetPtr;
}