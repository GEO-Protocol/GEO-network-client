#ifndef GEO_NETWORK_CLIENT_CYCLEFIVENODESINBETWEENMESSAGE_H
#define GEO_NETWORK_CLIENT_CYCLEFIVENODESINBETWEENMESSAGE_H

#include "base/CyclesBaseFiveOrSixNodesInBetweenMessage.h"

class CyclesFiveNodesInBetweenMessage: public CycleBaseFiveOrSixNodesInBetweenMessage {
public:
    typedef shared_ptr<CyclesFiveNodesInBetweenMessage> Shared;

public:
    using CycleBaseFiveOrSixNodesInBetweenMessage::CycleBaseFiveOrSixNodesInBetweenMessage;

    const MessageType typeID() const {
        return Message::MessageType::Cycles_FiveNodesMiddleware;
    };
};
#endif //GEO_NETWORK_CLIENT_CYCLEFIVENODESINBETWEENMESSAGE_H