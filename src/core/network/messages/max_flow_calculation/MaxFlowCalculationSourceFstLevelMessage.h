#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCEFSTLEVELMESSAGE_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCEFSTLEVELMESSAGE_H

#include "../SenderMessage.h"

class MaxFlowCalculationSourceFstLevelMessage:
    public SenderMessage {

public:
    typedef shared_ptr<MaxFlowCalculationSourceFstLevelMessage> Shared;

public:
    using SenderMessage::SenderMessage;

    const MessageType typeID() const;
};

#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCEFSTLEVELMESSAGE_H
