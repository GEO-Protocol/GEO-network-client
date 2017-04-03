
#include "base/CycleBaseFiveOrSixNodesBoundaryMessage.h"
class CycleFiveNodesBoundaryMessage: public CycleBaseFiveOrSixNodesBoundaryMessage {
public:
    typedef shared_ptr<CycleSixNodesBoundaryMessage> Shared;

public:
    CycleFiveNodesBoundaryMessage(
        vector<NodeUUID> &path,
        const vector<pair<NodeUUID, TrustLineBalance>> &boundaryNodes) : CycleBaseFiveOrSixNodesBoundaryMessage(path,
                                                                                                                boundaryNodes) {};

    CycleFiveNodesBoundaryMessage(
        BytesShared buffer):CycleBaseFiveOrSixNodesBoundaryMessage(buffer){};

    const MessageType typeID() const {
        return Message::MessageTypeID::CycleFiveNodesBoundaryMessage;
    };
};

