#ifndef GEO_NETWORK_CLIENT_CYCLEFIVENODESBOUNDARYMESSAGE_H
#define GEO_NETWORK_CLIENT_CYCLEFIVENODESBOUNDARYMESSAGE_H

#include "base/CyclesBaseFiveOrSixNodesBoundaryMessage.h"

class CyclesFiveNodesBoundaryMessage:
    public CycleBaseFiveOrSixNodesBoundaryMessage {
public:
    CyclesFiveNodesBoundaryMessage(
        vector<NodeUUID> &path,
        const vector<pair<NodeUUID, TrustLineBalance>> &boundaryNodes) :
        CycleBaseFiveOrSixNodesBoundaryMessage(
            path,
            boundaryNodes){};

    CyclesFiveNodesBoundaryMessage(
        BytesShared buffer) :
        CycleBaseFiveOrSixNodesBoundaryMessage(buffer){};

    const MessageType typeID() const {
        return Message::MessageTypeID::Cycles_FiveNodesBoundaryMessage;
    };
};
#endif //GEO_NETWORK_CLIENT_CYCLEFIVENODESBOUNDARYMESSAGE_H
