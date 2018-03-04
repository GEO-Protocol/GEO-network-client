#ifndef GEO_NETWORK_CLIENT_CYCLESIXNODESBOUNDARYMESSAGE_H
#define GEO_NETWORK_CLIENT_CYCLESIXNODESBOUNDARYMESSAGE_H

#include "base/CyclesBaseFiveOrSixNodesBoundaryMessage.h"
class CyclesSixNodesBoundaryMessage:
        public CyclesBaseFiveOrSixNodesBoundaryMessage {
public:
    typedef shared_ptr<CyclesSixNodesBoundaryMessage> Shared;

public:
    CyclesSixNodesBoundaryMessage(
        const SerializedEquivalent equivalent,
        vector<NodeUUID> &path,
        vector<NodeUUID> &boundaryNodes) :
        CyclesBaseFiveOrSixNodesBoundaryMessage(
            equivalent,
            path,
            boundaryNodes) {};

    CyclesSixNodesBoundaryMessage(
        BytesShared buffer):
        CyclesBaseFiveOrSixNodesBoundaryMessage(
            buffer){};

    const MessageType typeID() const {
        return Message::MessageType::Cycles_SixNodesBoundary;
    };
};

#endif //GEO_NETWORK_CLIENT_CYCLESIXNODESBOUNDARYMESSAGE_H
