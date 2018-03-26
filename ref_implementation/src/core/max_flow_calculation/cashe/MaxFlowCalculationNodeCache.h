/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

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
