//
// Created by mc on 16.02.17.
//

#ifndef GEO_NETWORK_CLIENT_SENDMAXFLOWCALCULATIONTARGETFSTLEVELMESSAGE_H
#define GEO_NETWORK_CLIENT_SENDMAXFLOWCALCULATIONTARGETFSTLEVELMESSAGE_H

#include "../../base/max_flow_calculation/MaxFlowCalculationMessage.h"

class SendMaxFlowCalculationTargetFstLevelMessage : public MaxFlowCalculationMessage {

public:
    typedef shared_ptr<SendMaxFlowCalculationTargetFstLevelMessage> Shared;

public:
    SendMaxFlowCalculationTargetFstLevelMessage(
        NodeUUID &targetUUID);

    const MessageType typeID() const;

};


#endif //GEO_NETWORK_CLIENT_SENDMAXFLOWCALCULATIONTARGETFSTLEVELMESSAGE_H
