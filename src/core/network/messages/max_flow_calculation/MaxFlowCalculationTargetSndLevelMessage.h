#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETSNDLEVELMESSAGE_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETSNDLEVELMESSAGE_H

#include "../base/max_flow_calculation/MaxFlowCalculationMessage.h"

class MaxFlowCalculationTargetSndLevelMessage : public MaxFlowCalculationMessage {

public:
    typedef shared_ptr<MaxFlowCalculationTargetSndLevelMessage> Shared;

public:
    MaxFlowCalculationTargetSndLevelMessage(
        const NodeUUID& senderUUID,
        const NodeUUID& targetUUID);

    MaxFlowCalculationTargetSndLevelMessage(
        BytesShared buffer);

    const MessageType typeID() const;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETSNDLEVELMESSAGE_H
