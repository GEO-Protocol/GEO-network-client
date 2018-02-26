#include "TopologyTrustLineWithPtr.h"

TopologyTrustLineWithPtr::TopologyTrustLineWithPtr(
    const TopologyTrustLine::Shared maxFlowCalculationTrustLine,
    unordered_set<TopologyTrustLineWithPtr*>* hashMapPtr) :

    mTopologyTrustLine(maxFlowCalculationTrustLine),
    mHashSetPtr(hashMapPtr)
{}

TopologyTrustLine::Shared TopologyTrustLineWithPtr::topologyTrustLine()
{
    return mTopologyTrustLine;
}

unordered_set<TopologyTrustLineWithPtr*>* TopologyTrustLineWithPtr::hashSetPtr()
{
    return mHashSetPtr;
}