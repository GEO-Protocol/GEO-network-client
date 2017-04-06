#ifndef GEO_NETWORK_CLIENT_CYCLESIXNODESBOUNDARYMESSAGE_H
#define GEO_NETWORK_CLIENT_CYCLESIXNODESBOUNDARYMESSAGE_H

#include "base/CyclesBaseFiveOrSixNodesBoundaryMessage.h"
class CyclesSixNodesBoundaryMessage: public CycleBaseFiveOrSixNodesBoundaryMessage {
public:
    typedef shared_ptr<CyclesSixNodesBoundaryMessage> Shared;

public:
    CyclesSixNodesBoundaryMessage(
        vector<NodeUUID> &path,
        const vector<pair<NodeUUID, TrustLineBalance>> &boundaryNodes) : CycleBaseFiveOrSixNodesBoundaryMessage(path,
                                                                                                      boundaryNodes) {};

    CyclesSixNodesBoundaryMessage(
        BytesShared buffer):CycleBaseFiveOrSixNodesBoundaryMessage(buffer){};

    const MessageType typeID() const {
        return Message::MessageTypeID::Cycles_SixNodesBoundaryMessage;
    };
};

#endif //GEO_NETWORK_CLIENT_CYCLESIXNODESBOUNDARYMESSAGE_H
