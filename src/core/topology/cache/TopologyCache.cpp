#include "TopologyCache.h"

TopologyCache::TopologyCache(
    const vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> &outgoingFlows,
    const vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> &incomingFlows)
{
    for (auto &nodeAddressAndFlow : outgoingFlows) {
        mOutgoingFlows.push_back(nodeAddressAndFlow);
    }
    for (auto &nodeAddressAndFlow : incomingFlows) {
        mIncomingFlows.push_back(nodeAddressAndFlow);
    }
}

// check if incoming flow already cached
bool TopologyCache::containsIncomingFlow(
    BaseAddress::Shared nodeAddress,
    ConstSharedTrustLineAmount flow)
{
    auto nodeAddressAndIncomingFlowIt = mIncomingFlows.begin();
    while (nodeAddressAndIncomingFlowIt != mIncomingFlows.end()) {
        if (nodeAddressAndIncomingFlowIt->first == nodeAddress) {
            break;
        }
        nodeAddressAndIncomingFlowIt++;
    }
    // if not present then insert
    if (nodeAddressAndIncomingFlowIt == mIncomingFlows.end()) {
        if (*flow == TrustLine::kZeroAmount()) {
            return true;
        } else {
            mIncomingFlows.emplace_back(
                nodeAddress,
                flow);
            return false;
        }
    } else {
        auto sharedFlow = (*nodeAddressAndIncomingFlowIt).second;
        // if flow present but now it is zero, then delete
        if (*flow.get() == TrustLine::kZeroAmount()) {
            mIncomingFlows.erase(nodeAddressAndIncomingFlowIt);
            return false;
        }
        // if flow differs then update
        if (*sharedFlow.get() != *flow.get()) {
            mIncomingFlows.erase(nodeAddressAndIncomingFlowIt);
            mIncomingFlows.emplace_back(
                nodeAddress,
                flow);
            return false;
        }
    }
    return true;
}

// check if outgoing flow already cached
bool TopologyCache::containsOutgoingFlow(
    BaseAddress::Shared nodeAddress,
    const ConstSharedTrustLineAmount flow)
{
    auto nodeAddressAndOutgoingFlowIt = mOutgoingFlows.begin();
    while (nodeAddressAndOutgoingFlowIt != mOutgoingFlows.end()) {
        if (nodeAddressAndOutgoingFlowIt->first == nodeAddress) {
            break;
        }
        nodeAddressAndOutgoingFlowIt++;
    }
    // if not present then insert
    if (nodeAddressAndOutgoingFlowIt == mOutgoingFlows.end()) {
        if (*flow == TrustLine::kZeroAmount()) {
            return true;
        } else {
            mOutgoingFlows.emplace_back(
                nodeAddress,
                flow);
            return false;
        }
    } else {
        auto sharedFlow = (*nodeAddressAndOutgoingFlowIt).second;
        // if flow present but now it is zero, then delete
        if (*flow.get() == TrustLine::kZeroAmount()) {
            mOutgoingFlows.erase(nodeAddressAndOutgoingFlowIt);
            return false;
        }
        // if flow differs then update
        if (*sharedFlow.get() != *flow.get()) {
            mOutgoingFlows.erase(nodeAddressAndOutgoingFlowIt);
            mOutgoingFlows.emplace_back(
               nodeAddress,
               flow);
            return false;
        }
    }
    return true;
}