#ifndef GEO_NETWORK_CLIENT_RESULTMAXFLOWCALCULATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_RESULTMAXFLOWCALCULATIONMESSAGE_H

#include "../base/max_flow_calculation/MaxFlowCalculationConfirmationMessage.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"

#include <vector>

class ResultMaxFlowCalculationMessage:
    public MaxFlowCalculationConfirmationMessage {

public:
    typedef shared_ptr<ResultMaxFlowCalculationMessage> Shared;

public:
    ResultMaxFlowCalculationMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID& senderUUID,
        vector<BaseAddress::Shared> senderAddresses,
        vector<pair<NodeUUID, ConstSharedTrustLineAmount>> &outgoingFlows,
        vector<pair<NodeUUID, ConstSharedTrustLineAmount>> &incomingFlows,
        vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> &outgoingFlowsNew,
        vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> &incomingFlowsNew);

    ResultMaxFlowCalculationMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    const bool isAddToConfirmationNotStronglyRequiredMessagesHandler() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const
        throw(bad_alloc);

    const vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlows() const;

    const vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlows() const;

    const vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> outgoingFlowsNew() const;

    const vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> incomingFlowsNew() const;

private:
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> mOutgoingFlows;
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> mIncomingFlows;
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> mOutgoingFlowsNew;
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> mIncomingFlowsNew;
};


#endif //GEO_NETWORK_CLIENT_RESULTMAXFLOWCALCULATIONMESSAGE_H
