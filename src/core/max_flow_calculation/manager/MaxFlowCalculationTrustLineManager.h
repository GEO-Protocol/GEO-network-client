//
// Created by mc on 19.02.17.
//

#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINEMANAGER_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINEMANAGER_H

#include "../../common/NodeUUID.h"
#include "../MaxFlowCalculationTrustLine.h"
#include "../MaxFlowCalculationNodeEntity.h"

#include <map>
#include <vector>

#include "boost/container/flat_set.hpp"

using namespace std;
class MaxFlowCalculationTrustLineManager {

public:

    void addTrustLine(MaxFlowCalculationTrustLine::Shared trustLine);

    void addIncomingFlow(
        const NodeUUID& node1UUID,
        const NodeUUID& node2UUID,
        const TrustLineAmount& flow);

    void addOutgoingFlow(
        const NodeUUID& node1UUID,
        const NodeUUID& node2UUID,
        const TrustLineAmount& flow);

    void addFlow(
        const NodeUUID& node1UUID,
        const NodeUUID& node2UUID,
        const TrustLineAmount& flow);

// todo make private after testing
public:
    map<NodeUUID, vector<MaxFlowCalculationTrustLine::Shared>> mvTrustLines;
    map<NodeUUID, MaxFlowCalculationNodeEntity::Shared> mEntities;

};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRUSTLINEMANAGER_H
