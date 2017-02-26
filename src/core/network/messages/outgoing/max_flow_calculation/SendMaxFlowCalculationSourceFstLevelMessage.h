#ifndef GEO_NETWORK_CLIENT_SENDMAXFLOWCALCULATIONSOURCEFSTLEVELMESSAGE_H
#define GEO_NETWORK_CLIENT_SENDMAXFLOWCALCULATIONSOURCEFSTLEVELMESSAGE_H

#include "../../base/max_flow_calculation/MaxFlowCalculationMessage.h"

class SendMaxFlowCalculationSourceFstLevelMessage : public  MaxFlowCalculationMessage{

public:
    typedef shared_ptr<SendMaxFlowCalculationSourceFstLevelMessage> Shared;

public:
    SendMaxFlowCalculationSourceFstLevelMessage(
        const NodeUUID& senderUUID,
        const NodeUUID& targetUUID);

    const MessageType typeID() const;
};


#endif //GEO_NETWORK_CLIENT_SENDMAXFLOWCALCULATIONSOURCEFSTLEVELMESSAGE_H
