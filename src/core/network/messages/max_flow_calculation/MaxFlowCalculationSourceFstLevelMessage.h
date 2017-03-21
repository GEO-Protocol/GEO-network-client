#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCEFSTLEVELMESSAGE_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCEFSTLEVELMESSAGE_H

#include "../SenderMessage.h"

class MaxFlowCalculationSourceFstLevelMessage : public SenderMessage {

public:
    typedef shared_ptr<MaxFlowCalculationSourceFstLevelMessage> Shared;

public:

    MaxFlowCalculationSourceFstLevelMessage(
        const NodeUUID& senderUUID);

    MaxFlowCalculationSourceFstLevelMessage(
        BytesShared buffer);

    const MessageType typeID() const;

};

#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCEFSTLEVELMESSAGE_H
