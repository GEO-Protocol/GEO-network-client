#include "SecondLevelRoutingTableOutgoingMessage.h"

SecondLevelRoutingTableOutgoingMessage::SecondLevelRoutingTableOutgoingMessage(
    const NodeUUID &senderUUID) :

    RoutingTableOutgoingMessage(
        senderUUID) {}

const Message::MessageType SecondLevelRoutingTableOutgoingMessage::typeID() const {

    return Message::MessageTypeID::SecondLevelRoutingTableOutgoingMessageType;
}
