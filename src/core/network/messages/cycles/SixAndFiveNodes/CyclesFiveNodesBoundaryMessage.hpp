#ifndef GEO_NETWORK_CLIENT_CYCLEFIVENODESBOUNDARYMESSAGE_H
#define GEO_NETWORK_CLIENT_CYCLEFIVENODESBOUNDARYMESSAGE_H

#include "base/CyclesBaseFiveOrSixNodesBoundaryMessage.h"

class CyclesFiveNodesBoundaryMessage:
    public CyclesBaseFiveOrSixNodesBoundaryMessage {
public:
    CyclesFiveNodesBoundaryMessage(
        vector<NodeUUID> &path,
        vector<NodeUUID> &boundaryNodes) :
        CyclesBaseFiveOrSixNodesBoundaryMessage(
            path,
            boundaryNodes){};

    CyclesFiveNodesBoundaryMessage(
        BytesShared buffer) :
        CyclesBaseFiveOrSixNodesBoundaryMessage(buffer){};

    const MessageType typeID() const {
        return Message::MessageTypeID::Cycles_FiveNodesBoundaryMessage;
    };
};
#endif //GEO_NETWORK_CLIENT_CYCLEFIVENODESBOUNDARYMESSAGE_H
