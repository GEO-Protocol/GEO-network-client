#ifndef GEO_NETWORK_CLIENT_TOPOLOGYTRUSTLINEWITHPTR_H
#define GEO_NETWORK_CLIENT_TOPOLOGYTRUSTLINEWITHPTR_H

#include "../TopologyTrustLine.h"
#include <unordered_set>

class TopologyTrustLineWithPtr {

public:
    TopologyTrustLineWithPtr(
        const TopologyTrustLine::Shared maxFlowCalculationTrustLine,
        unordered_set<TopologyTrustLineWithPtr*>* hashMapPtr);

    TopologyTrustLine::Shared topologyTrustLine();

    unordered_set<TopologyTrustLineWithPtr*>* hashSetPtr();

private:
    TopologyTrustLine::Shared mTopologyTrustLine;
    unordered_set<TopologyTrustLineWithPtr*>* mHashSetPtr;
};


#endif //GEO_NETWORK_CLIENT_TOPOLOGYTRUSTLINEWITHPTR_H
