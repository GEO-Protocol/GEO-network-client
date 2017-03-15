#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCESNDLEVELMESSAGE_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCESNDLEVELMESSAGE_H

#include "../base/max_flow_calculation/MaxFlowCalculationMessage.h"

class MaxFlowCalculationSourceSndLevelMessage : public MaxFlowCalculationMessage {

public:
    typedef shared_ptr<MaxFlowCalculationSourceSndLevelMessage> Shared;

public:

    MaxFlowCalculationSourceSndLevelMessage(
            const NodeUUID& senderUUID,
            const NodeUUID& targetUUID);

    MaxFlowCalculationSourceSndLevelMessage(
        BytesShared buffer);

    const MessageType typeID() const;

};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCESNDLEVELMESSAGE_H
