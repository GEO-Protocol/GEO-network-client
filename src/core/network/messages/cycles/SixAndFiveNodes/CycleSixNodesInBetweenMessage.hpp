#include "base/CycleBaseFiveOrSixNodesInBetweenMessage.h"

class CycleSixNodesInBetweenMessage: public CycleBaseFiveOrSixNodesInBetweenMessage {
public:
    typedef shared_ptr<CycleSixNodesInBetweenMessage> Shared;
public:
    CycleSixNodesInBetweenMessage(){};
    CycleSixNodesInBetweenMessage(
        vector<NodeUUID> &path):CycleBaseFiveOrSixNodesInBetweenMessage(path){};
    CycleSixNodesInBetweenMessage(
        BytesShared buffer):CycleBaseFiveOrSixNodesInBetweenMessage(buffer){};

    const MessageType typeID() const {
        return Message::MessageTypeID::CycleSixNodesInBetweenMessage;
    };
};
