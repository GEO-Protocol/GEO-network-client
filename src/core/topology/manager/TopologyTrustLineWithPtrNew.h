#ifndef GEO_NETWORK_CLIENT_TOPOLOGYTRUSTLINEWITHPTRNEW_H
#define GEO_NETWORK_CLIENT_TOPOLOGYTRUSTLINEWITHPTRNEW_H

#include "../TopologyTrustLineNew.h"
#include <unordered_set>

class TopologyTrustLineWithPtrNew {

public:
    TopologyTrustLineWithPtrNew(
        const TopologyTrustLineNew::Shared maxFlowCalculationTrustLine,
        unordered_set<TopologyTrustLineWithPtrNew*>* hashMapPtr);

    TopologyTrustLineNew::Shared topologyTrustLine();

    unordered_set<TopologyTrustLineWithPtrNew*>* hashSetPtr();

private:
    TopologyTrustLineNew::Shared mTopologyTrustLine;
    unordered_set<TopologyTrustLineWithPtrNew*>* mHashSetPtr;
};


#endif //GEO_NETWORK_CLIENT_TOPOLOGYTRUSTLINEWITHPTRNEW_H
