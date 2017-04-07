#ifndef GEO_NETWORK_CLIENT_CYCLESIXNODESBOUNDARYMESSAGE_H
#define GEO_NETWORK_CLIENT_CYCLESIXNODESBOUNDARYMESSAGE_H

#include "base/CyclesBaseFiveOrSixNodesBoundaryMessage.h"
class CyclesSixNodesBoundaryMessage:
        public CyclesBaseFiveOrSixNodesBoundaryMessage {
public:
    typedef shared_ptr<CyclesSixNodesBoundaryMessage> Shared;

public:
    CyclesSixNodesBoundaryMessage(
        vector<NodeUUID> &path,
        vector<NodeUUID> &boundaryNodes) :
            CyclesBaseFiveOrSixNodesBoundaryMessage(
                    path,
                    boundaryNodes) {};

    CyclesSixNodesBoundaryMessage(
        BytesShared buffer):CyclesBaseFiveOrSixNodesBoundaryMessage(buffer){};

    const MessageType typeID() const {
        return Message::MessageTypeID::Cycles_SixNodesBoundaryMessage;
    };
};

#endif //GEO_NETWORK_CLIENT_CYCLESIXNODESBOUNDARYMESSAGE_H
