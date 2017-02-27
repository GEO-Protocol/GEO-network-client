//
// Created by mc on 19.02.17.
//

#include "MaxFlowCalculationTrustLineManager.h"

void MaxFlowCalculationTrustLineManager::addTrustLine(MaxFlowCalculationTrustLine::Shared trustLine) {
    auto const &it = mvTrustLines.find(trustLine->getSourceUUID());
    if (it == mvTrustLines.end()) {
        vector<MaxFlowCalculationTrustLine::Shared> newVect;
        newVect.push_back(trustLine);
        mvTrustLines.insert(make_pair(trustLine->getSourceUUID(), newVect));
    } else {
        it->second.push_back(trustLine);
    }
}

void MaxFlowCalculationTrustLineManager::addIncomingFlow(
    const NodeUUID &node1UUID,
    const NodeUUID &node2UUID,
    const TrustLineAmount &flow) {

    auto const &it = mEntities.find(node1UUID);
    if (it == mEntities.end()) {
        auto newEntity = make_shared<MaxFlowCalculationNodeEntity>(
            node1UUID);
        newEntity->addIncomingFlow(node2UUID, flow);
        mEntities.insert(make_pair(node1UUID, newEntity));
    } else {
        it->second->addIncomingFlow(node2UUID, flow);
    }
}

void MaxFlowCalculationTrustLineManager::addOutgoingFlow(
    const NodeUUID &node1UUID,
    const NodeUUID &node2UUID,
    const TrustLineAmount &flow) {

    auto const &it = mEntities.find(node1UUID);
    if (it == mEntities.end()) {
        auto newEntity = make_shared<MaxFlowCalculationNodeEntity>(
            node1UUID);
        newEntity->addOutgoingFlow(node2UUID, flow);
        mEntities.insert(make_pair(node1UUID, newEntity));
    } else {
        it->second->addOutgoingFlow(node2UUID, flow);
    }
}

void MaxFlowCalculationTrustLineManager::addFlow(
    const NodeUUID &node1UUID,
    const NodeUUID &node2UUID,
    const TrustLineAmount &flow) {

    addOutgoingFlow(node1UUID, node2UUID, flow);
    addOutgoingFlow(node2UUID, node1UUID, TrustLineAmount(0));
    addIncomingFlow(node2UUID, node1UUID, flow);
    addIncomingFlow(node1UUID, node2UUID, TrustLineAmount(0));
}

vector<MaxFlowCalculationTrustLine::Shared> MaxFlowCalculationTrustLineManager::getSortedTrustLines(
    const NodeUUID& nodeUUID) {

    vector<MaxFlowCalculationTrustLine::Shared> result;
    auto const &nodeUUIDAndVector = mvTrustLines.find(nodeUUID);
    if (nodeUUIDAndVector == mvTrustLines.end()) {
        return result;
    }

    for (auto const &trustLine : nodeUUIDAndVector->second) {
        result.push_back(trustLine);
    }

    // sort using a custom function object
    struct {
        bool operator()(
            MaxFlowCalculationTrustLine::Shared a,
            MaxFlowCalculationTrustLine::Shared b) {
            auto aTrustLineFreeAmountPtr = a.get()->getFreeAmount();
            auto bTrustLineFreeAmountPtr = b.get()->getFreeAmount();
            return *aTrustLineFreeAmountPtr > *bTrustLineFreeAmountPtr;
        }
    } customLess;
    std::sort(result.begin(), result.end(), customLess);
    return result;
}