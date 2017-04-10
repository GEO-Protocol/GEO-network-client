#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETFSTLEVELMESSAGE_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETFSTLEVELMESSAGE_H

#include "../base/max_flow_calculation/MaxFlowCalculationMessage.h"

class MaxFlowCalculationTargetFstLevelMessage : public MaxFlowCalculationMessage {

public:
    typedef shared_ptr<MaxFlowCalculationTargetFstLevelMessage> Shared;

public:

    MaxFlowCalculationTargetFstLevelMessage(
        const NodeUUID& senderUUID,
        const NodeUUID& targetUUID);

    MaxFlowCalculationTargetFstLevelMessage(
        BytesShared buffer);

    const MessageType typeID() const;

};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETFSTLEVELMESSAGE_H
