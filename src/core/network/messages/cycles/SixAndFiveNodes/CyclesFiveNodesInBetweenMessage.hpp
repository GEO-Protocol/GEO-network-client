#ifndef GEO_NETWORK_CLIENT_CYCLEFIVENODESINBETWEENMESSAGE_H
#define GEO_NETWORK_CLIENT_CYCLEFIVENODESINBETWEENMESSAGE_H

#include "base/CyclesBaseFiveOrSixNodesInBetweenMessage.h"

class CyclesFiveNodesInBetweenMessage: public CycleBaseFiveOrSixNodesInBetweenMessage {
public:
    typedef shared_ptr<CyclesFiveNodesInBetweenMessage> Shared;
public:
    CyclesFiveNodesInBetweenMessage(
        const SerializedEquivalent equivalent,
        vector<NodeUUID> &path):
        CycleBaseFiveOrSixNodesInBetweenMessage(
            equivalent,
            path){};

    CyclesFiveNodesInBetweenMessage(
        BytesShared buffer):
        CycleBaseFiveOrSixNodesInBetweenMessage(
            buffer){};

    const MessageType typeID() const {
        return Message::MessageType::Cycles_FiveNodesMiddleware;
    };
};
#endif //GEO_NETWORK_CLIENT_CYCLEFIVENODESINBETWEENMESSAGE_H