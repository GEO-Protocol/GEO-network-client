/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHE_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHE_H

#include "../../common/Types.h"
#include "../../common/NodeUUID.h"
#include "../../trust_lines/TrustLine.h"
#include "../../common/time/TimeUtils.h"

#include <unordered_map>
#include <vector>
#include <boost/functional/hash.hpp>

class MaxFlowCalculationCache {

public:
    typedef shared_ptr<MaxFlowCalculationCache> Shared;

    MaxFlowCalculationCache(
        const vector<pair<NodeUUID, ConstSharedTrustLineAmount>> &outgoingFlows,
        const vector<pair<NodeUUID, ConstSharedTrustLineAmount>> &incomingFlows);

    bool containsIncomingFlow(
        const NodeUUID &nodeUUID,
        ConstSharedTrustLineAmount flow);

    bool containsOutgoingFlow(
        const NodeUUID &nodeUUID,
        ConstSharedTrustLineAmount flow);

private:
    unordered_map<NodeUUID, ConstSharedTrustLineAmount, boost::hash<boost::uuids::uuid>> mIncomingFlows;
    unordered_map<NodeUUID, ConstSharedTrustLineAmount, boost::hash<boost::uuids::uuid>> mOutgoingFlows;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHE_H
