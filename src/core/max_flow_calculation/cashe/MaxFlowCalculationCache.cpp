#include "MaxFlowCalculationCache.h"

MaxFlowCalculationCache::MaxFlowCalculationCache(
    const NodeUUID &nodeUUID,
    const set<NodeUUID> outgoingUUIDs,
    const set<NodeUUID> incomingUUIDs) :

    mNodeUUID(nodeUUID),
    mOutgoingUUIDs(outgoingUUIDs),
    mIncomingUUIDs(incomingUUIDs),
    mTimeStampCreated(utc_now()) {}

void MaxFlowCalculationCache::addIncomingUUID(
    const NodeUUID &nodeUUID) {

    mIncomingUUIDs.insert(nodeUUID);
}

void MaxFlowCalculationCache::addOutgoingUUID(
    const NodeUUID &nodeUUID) {

    mOutgoingUUIDs.insert(nodeUUID);
}

bool MaxFlowCalculationCache::containsIncomingUUID(
    const NodeUUID& nodeUUID) const {

    if (mIncomingUUIDs.find(nodeUUID) == mIncomingUUIDs.end()) {
        return false;
    } else {
        return true;
    }
}

bool MaxFlowCalculationCache::containsOutgoingUUID(
    const NodeUUID& nodeUUID) const {

    if (mOutgoingUUIDs.find(nodeUUID) == mOutgoingUUIDs.end()) {
        return false;
    } else {
        return true;
    }
}

const DateTime& MaxFlowCalculationCache::timeStampCreated() const {

    return mTimeStampCreated;
}

const NodeUUID& MaxFlowCalculationCache::nodeUUID() const {

    return mNodeUUID;
}
