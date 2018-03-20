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
        vector<pair<NodeUUID, ConstSharedTrustLineAmount>> &outgoingFlows,
        vector<pair<NodeUUID, ConstSharedTrustLineAmount>> &incomingFlows);

    ResultMaxFlowCalculationMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    const bool isAddToConfirmationNotStronglyRequiredMessagesHandler() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const
        throw(bad_alloc);

    const vector<pair<NodeUUID, ConstSharedTrustLineAmount>> outgoingFlows() const;

    const vector<pair<NodeUUID, ConstSharedTrustLineAmount>> incomingFlows() const;

private:
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> mOutgoingFlows;
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> mIncomingFlows;
};


#endif //GEO_NETWORK_CLIENT_RESULTMAXFLOWCALCULATIONMESSAGE_H
