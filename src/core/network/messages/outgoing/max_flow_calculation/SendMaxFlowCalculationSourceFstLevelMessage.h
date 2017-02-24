//
// Created by mc on 16.02.17.
//

#ifndef GEO_NETWORK_CLIENT_SENDMAXFLOWCALCULATIONSOURCEFSTLEVELMESSAGE_H
#define GEO_NETWORK_CLIENT_SENDMAXFLOWCALCULATIONSOURCEFSTLEVELMESSAGE_H

#include "../../base/max_flow_calculation/MaxFlowCalculationMessage.h"

class SendMaxFlowCalculationSourceFstLevelMessage : public  MaxFlowCalculationMessage{

public:
    typedef shared_ptr<SendMaxFlowCalculationSourceFstLevelMessage> Shared;

public:
    SendMaxFlowCalculationSourceFstLevelMessage(
        NodeUUID &targetUUID);

    const MessageType typeID() const;
};


#endif //GEO_NETWORK_CLIENT_SENDMAXFLOWCALCULATIONSOURCEFSTLEVELMESSAGE_H
