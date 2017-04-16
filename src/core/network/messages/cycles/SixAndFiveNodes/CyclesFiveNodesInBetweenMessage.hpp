#ifndef GEO_NETWORK_CLIENT_CYCLEFIVENODESINBETWEENMESSAGE_H
#define GEO_NETWORK_CLIENT_CYCLEFIVENODESINBETWEENMESSAGE_H

#include "base/CyclesBaseFiveOrSixNodesInBetweenMessage.h"

class CyclesFiveNodesInBetweenMessage: public CycleBaseFiveOrSixNodesInBetweenMessage {
public:
    typedef shared_ptr<CyclesFiveNodesInBetweenMessage> Shared;
public:
    CyclesFiveNodesInBetweenMessage(){};
    CyclesFiveNodesInBetweenMessage(
        vector<NodeUUID> &path):CycleBaseFiveOrSixNodesInBetweenMessage(path){};
    CyclesFiveNodesInBetweenMessage(
        BytesShared buffer):CycleBaseFiveOrSixNodesInBetweenMessage(buffer){};

    const MessageType typeID() const {
        return Message::MessageTypeID::Cycles_FiveNodesInBetweenMessage;
    };
};
#endif //GEO_NETWORK_CLIENT_CYCLEFIVENODESINBETWEENMESSAGE_H