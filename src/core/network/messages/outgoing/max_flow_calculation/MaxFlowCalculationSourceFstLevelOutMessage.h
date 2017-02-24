//
// Created by mc on 17.02.17.
//

#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCEFSTLEVELOUTMESSAGE_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCEFSTLEVELOUTMESSAGE_H

#include "../../base/max_flow_calculation/MaxFlowCalculationMessage.h"

class MaxFlowCalculationSourceFstLevelOutMessage : public MaxFlowCalculationMessage {

public:
    typedef shared_ptr<MaxFlowCalculationSourceFstLevelOutMessage> Shared;

public:
    MaxFlowCalculationSourceFstLevelOutMessage(
        NodeUUID &targetUUID);

    const MessageType typeID() const;

};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCEFSTLEVELOUTMESSAGE_H
