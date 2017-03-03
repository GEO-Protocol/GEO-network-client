#include "MaxFlowCalculationCache.h"

MaxFlowCalculationCache::MaxFlowCalculationCache(
    const NodeUUID &nodeUUID,
    const map<NodeUUID, TrustLineAmount> outgoingUUIDs,
    const map<NodeUUID, TrustLineAmount> incomingUUIDs) :

    mNodeUUID(nodeUUID),
    mOutgoingFlows(outgoingUUIDs),
    mIncomingFlows(incomingUUIDs),
    mTimeStampCreated(utc_now()) {}

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

const NodeUUID& MaxFlowCalculationCache::nodeUUID() const {

    return mNodeUUID;
}
