#ifndef GEO_NETWORK_CLIENT_TOPOLOGYCACHE_H
#define GEO_NETWORK_CLIENT_TOPOLOGYCACHE_H

#include "../../common/Types.h"
#include "../../contractors/addresses/BaseAddress.h"
#include "../../trust_lines/TrustLine.h"
#include "../../common/time/TimeUtils.h"

#include <vector>

class TopologyCache {

public:
    typedef shared_ptr<TopologyCache> Shared;

    TopologyCache(
        const vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> &outgoingFlows,
        const vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> &incomingFlows);

    bool containsIncomingFlow(
        BaseAddress::Shared nodeAddress,
        ConstSharedTrustLineAmount flow);

    bool containsOutgoingFlow(
        BaseAddress::Shared nodeAddress,
        ConstSharedTrustLineAmount flow);

private:
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> mIncomingFlows;
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> mOutgoingFlows;
};


#endif //GEO_NETWORK_CLIENT_TOPOLOGYCACHE_H
