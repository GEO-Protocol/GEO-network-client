#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETFSTLEVELMESSAGE_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETFSTLEVELMESSAGE_H

#include "../base/max_flow_calculation/MaxFlowCalculationMessage.h"

class MaxFlowCalculationTargetFstLevelMessage :
    public MaxFlowCalculationMessage {

public:
    typedef shared_ptr<MaxFlowCalculationTargetFstLevelMessage> Shared;

public:
    MaxFlowCalculationTargetFstLevelMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        ContractorID idOnReceiverSide,
        vector<BaseAddress::Shared> targetAddresses,
        bool isTargetGateway)
        noexcept;

    MaxFlowCalculationTargetFstLevelMessage(
        BytesShared buffer)
        noexcept;

    bool isTargetGateway() const;

    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes() const
    throw(bad_alloc);

private:
    bool mIsTargetGateway;
};

#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETFSTLEVELMESSAGE_H
