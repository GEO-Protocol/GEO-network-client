#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONNODECACHE_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONNODECACHE_H

#include "../../common/Types.h"
#include "../../common/time/TimeUtils.h"

class MaxFlowCalculationNodeCache {
public:
    typedef shared_ptr<MaxFlowCalculationNodeCache> Shared;

public:
    MaxFlowCalculationNodeCache(
        const TrustLineAmount &amount,
        bool isFinal = false);

    TrustLineAmount& currentFlow();

    bool isFlowFinal();

    DateTime lastModified();

    void updateCurrentFlow(
        const TrustLineAmount &amount,
        bool isFinal = false);

private:
    TrustLineAmount mCurrentFlow;
    bool mFinalFlow;
    DateTime mTimeLastModified;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONNODECACHE_H
