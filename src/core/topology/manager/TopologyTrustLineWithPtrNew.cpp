#include "TopologyTrustLineWithPtrNew.h"

TopologyTrustLineWithPtrNew::TopologyTrustLineWithPtrNew(
    const TopologyTrustLineNew::Shared maxFlowCalculationTrustLine,
    unordered_set<TopologyTrustLineWithPtrNew*>* hashMapPtr) :

    mTopologyTrustLine(maxFlowCalculationTrustLine),
    mHashSetPtr(hashMapPtr)
{}

TopologyTrustLineNew::Shared TopologyTrustLineWithPtrNew::topologyTrustLine()
{
    return mTopologyTrustLine;
}

unordered_set<TopologyTrustLineWithPtrNew*>* TopologyTrustLineWithPtrNew::hashSetPtr()
{
    return mHashSetPtr;
}