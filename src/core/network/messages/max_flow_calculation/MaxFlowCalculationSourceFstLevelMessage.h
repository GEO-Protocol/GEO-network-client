#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCEFSTLEVELMESSAGE_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCEFSTLEVELMESSAGE_H

#include "../base/max_flow_calculation/MaxFlowCalculationMessage.h"

class MaxFlowCalculationSourceFstLevelMessage : public MaxFlowCalculationMessage {

public:
    typedef shared_ptr<MaxFlowCalculationSourceFstLevelMessage> Shared;

public:

    MaxFlowCalculationSourceFstLevelMessage(
            const NodeUUID& senderUUID,
            const NodeUUID& targetUUID);

    MaxFlowCalculationSourceFstLevelMessage(
        BytesShared buffer);

    const MessageType typeID() const;

};

#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCEFSTLEVELMESSAGE_H
