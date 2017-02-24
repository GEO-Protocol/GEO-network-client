//
// Created by mc on 23.02.17.
//

#include "MaxFlowCalculationNodeEntity.h"

MaxFlowCalculationNodeEntity::MaxFlowCalculationNodeEntity(
    const NodeUUID& nodeUUID):

    mNodeUUID(nodeUUID) {}

void MaxFlowCalculationNodeEntity::addIncomingFlow(
    const NodeUUID& nodeUUID,
    const TrustLineAmount& flow) {

    mIncomingFlows.insert(make_pair(nodeUUID, flow));
}

void MaxFlowCalculationNodeEntity::addOutgoingFlow(
    const NodeUUID& nodeUUID,
    const TrustLineAmount& flow) {

    mOutgoingFlows.insert(make_pair(nodeUUID, flow));
}
