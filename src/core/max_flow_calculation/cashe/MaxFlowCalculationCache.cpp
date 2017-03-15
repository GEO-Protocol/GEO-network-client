#include "MaxFlowCalculationCache.h"

MaxFlowCalculationCache::MaxFlowCalculationCache(
    const vector<pair<NodeUUID, TrustLineAmount>> outgoingFlows,
    const vector<pair<NodeUUID, TrustLineAmount>> incomingFlows) {

    for (auto &nodeUUIDAndFlow : outgoingFlows) {
        mOutgoingFlows.insert(nodeUUIDAndFlow);
    }
    for (auto &nodeUUIDAndFlow : incomingFlows) {
        mIncomingFlows.insert(nodeUUIDAndFlow);
    }
}

bool MaxFlowCalculationCache::containsIncomingFlow(
    const NodeUUID &nodeUUID,
    const TrustLineAmount &flow) {

    auto nodeUUIDAndFlow = mIncomingFlows.find(nodeUUID);
    if (nodeUUIDAndFlow == mIncomingFlows.end()) {
        mIncomingFlows.insert(
            make_pair(
                nodeUUID,
                flow));
        return false;
    } else {
        if ((*nodeUUIDAndFlow).second != flow) {
            mIncomingFlows.erase(nodeUUIDAndFlow);
            mIncomingFlows.insert(
                make_pair(
                    nodeUUID,
                    flow));
            return false;
        } else {
            return true;
        }
    }
}

bool MaxFlowCalculationCache::containsOutgoingFlow(
    const NodeUUID &nodeUUID,
    const TrustLineAmount &flow) {

    auto nodeUUIDAndFlow = mOutgoingFlows.find(nodeUUID);
    if (nodeUUIDAndFlow == mOutgoingFlows.end()) {
        mOutgoingFlows.insert(
            make_pair(
                nodeUUID,
                flow));
        return false;
    } else {
        if ((*nodeUUIDAndFlow).second != flow) {
            mOutgoingFlows.erase(nodeUUIDAndFlow);
            mOutgoingFlows.insert(
                make_pair(
                    nodeUUID,
                    flow));
            return false;
        } else {
            return true;
        }
    }
}
