#ifndef GEO_NETWORK_CLIENT_CYCLEFIVENODESINBETWEENMESSAGE_H
#define GEO_NETWORK_CLIENT_CYCLEFIVENODESINBETWEENMESSAGE_H

#include "base/CyclesBaseFiveOrSixNodesInBetweenMessage.h"

class CycleFiveNodesInBetweenMessage: public CycleBaseFiveOrSixNodesInBetweenMessage {
public:
    typedef shared_ptr<CycleFiveNodesInBetweenMessage> Shared;
public:
    CycleFiveNodesInBetweenMessage(){};
    CycleFiveNodesInBetweenMessage(
        vector<NodeUUID> &path):CycleBaseFiveOrSixNodesInBetweenMessage(path){};
    CycleFiveNodesInBetweenMessage(
        BytesShared buffer):CycleBaseFiveOrSixNodesInBetweenMessage(buffer){};

    const MessageType typeID() const {
        return Message::MessageTypeID::Cycles_FiveNodesInBetweenMessage;
    };
};
#endif //GEO_NETWORK_CLIENT_CYCLEFIVENODESINBETWEENMESSAGE_H