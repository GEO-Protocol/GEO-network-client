//
// Created by mc on 23.02.17.
//

#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONNODEENTITY_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONNODEENTITY_H


#include "../common/NodeUUID.h"
#include "../common/Types.h"
#include <map>

class MaxFlowCalculationNodeEntity {

public:
    typedef shared_ptr<MaxFlowCalculationNodeEntity> Shared;
    typedef shared_ptr<const MaxFlowCalculationNodeEntity> SharedConst;

public:
    MaxFlowCalculationNodeEntity(
        const NodeUUID& nodeUUID);

    void addIncomingFlow(const NodeUUID& nodeUUID, const TrustLineAmount& flow);

    void addOutgoingFlow(const NodeUUID& nodeUUID, const TrustLineAmount& flow);

    //todo change on private after testing
public:
    NodeUUID mNodeUUID;
    map<NodeUUID, TrustLineAmount> mIncomingFlows;
    map<NodeUUID, TrustLineAmount> mOutgoingFlows;

};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONNODEENTITY_H
