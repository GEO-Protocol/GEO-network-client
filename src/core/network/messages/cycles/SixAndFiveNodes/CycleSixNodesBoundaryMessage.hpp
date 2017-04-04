#ifndef GEO_NETWORK_CLIENT_CYCLESIXNODESBOUNDARYMESSAGE_H
#define GEO_NETWORK_CLIENT_CYCLESIXNODESBOUNDARYMESSAGE_H

#include "base/CycleBaseFiveOrSixNodesBoundaryMessage.h"
class CycleSixNodesBoundaryMessage: public CycleBaseFiveOrSixNodesBoundaryMessage {
public:
    typedef shared_ptr<CycleSixNodesBoundaryMessage> Shared;

public:
    CycleSixNodesBoundaryMessage(
        vector<NodeUUID> &path,
        const vector<pair<NodeUUID, TrustLineBalance>> &boundaryNodes) : CycleBaseFiveOrSixNodesBoundaryMessage(path,
                                                                                                      boundaryNodes) {};

    CycleSixNodesBoundaryMessage(
        BytesShared buffer):CycleBaseFiveOrSixNodesBoundaryMessage(buffer){};

    const MessageType typeID() const {
        return Message::MessageTypeID::CycleSixNodesBoundaryMessage;
    };
};

#endif //GEO_NETWORK_CLIENT_CYCLESIXNODESBOUNDARYMESSAGE_H
