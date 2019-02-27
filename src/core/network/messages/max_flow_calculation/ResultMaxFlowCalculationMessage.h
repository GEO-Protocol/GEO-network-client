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
        vector<BaseAddress::Shared> senderAddresses,
        vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> &outgoingFlows,
        vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> &incomingFlows);

    ResultMaxFlowCalculationMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    const bool isAddToConfirmationNotStronglyRequiredMessagesHandler() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const override;

    const vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> outgoingFlows() const;

    const vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> incomingFlows() const;

private:
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> mOutgoingFlows;
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> mIncomingFlows;
};


#endif //GEO_NETWORK_CLIENT_RESULTMAXFLOWCALCULATIONMESSAGE_H
