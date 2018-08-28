/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

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
