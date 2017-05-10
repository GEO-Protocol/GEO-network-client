#include "MaxFlowCalculationCache.h"

MaxFlowCalculationCache::MaxFlowCalculationCache(
    const vector<pair<NodeUUID, ConstSharedTrustLineAmount>> &outgoingFlows,
    const vector<pair<NodeUUID, ConstSharedTrustLineAmount>> &incomingFlows)
{
    for (auto &nodeUUIDAndFlow : outgoingFlows) {
        mOutgoingFlows.insert(nodeUUIDAndFlow);
    }
    for (auto &nodeUUIDAndFlow : incomingFlows) {
        mIncomingFlows.insert(nodeUUIDAndFlow);
    }
}

bool MaxFlowCalculationCache::containsIncomingFlow(
    const NodeUUID &nodeUUID,
    ConstSharedTrustLineAmount flow)
{
    auto nodeUUIDAndFlow = mIncomingFlows.find(nodeUUID);
    if (nodeUUIDAndFlow == mIncomingFlows.end()) {
        mIncomingFlows.insert(
            make_pair(
                nodeUUID,
                flow));
        return false;
    } else {
        auto sharedFlow = (*nodeUUIDAndFlow).second;
        if (*sharedFlow.get() != *flow.get()) {
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
    const ConstSharedTrustLineAmount flow)
{
    auto nodeUUIDAndFlow = mOutgoingFlows.find(nodeUUID);
    if (nodeUUIDAndFlow == mOutgoingFlows.end()) {
        mOutgoingFlows.insert(
            make_pair(
                nodeUUID,
                flow));
        return false;
    } else {
        auto sharedFlow = (*nodeUUIDAndFlow).second;
        if (*sharedFlow.get() != *flow.get()) {
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
