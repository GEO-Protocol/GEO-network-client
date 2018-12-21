#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETSNDLEVELMESSAGE_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETSNDLEVELMESSAGE_H

#include "../base/max_flow_calculation/MaxFlowCalculationMessage.h"

class MaxFlowCalculationTargetSndLevelMessage : public MaxFlowCalculationMessage {

public:
    typedef shared_ptr<MaxFlowCalculationTargetSndLevelMessage> Shared;

public:
    MaxFlowCalculationTargetSndLevelMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        ContractorID idOnReceiverSide,
        vector<BaseAddress::Shared> targetAddresses,
        bool isTargetGateway)
        noexcept;

    MaxFlowCalculationTargetSndLevelMessage(
        BytesShared buffer)
        noexcept;

    bool isTargetGateway() const;

    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes() const
        throw(bad_alloc);

private:
    bool mIsTargetGateway;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETSNDLEVELMESSAGE_H
