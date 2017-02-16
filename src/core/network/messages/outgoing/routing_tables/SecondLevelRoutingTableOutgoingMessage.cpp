#include "SecondLevelRoutingTableOutgoingMessage.h"

SecondLevelRoutingTableOutgoingMessage::SecondLevelRoutingTableOutgoingMessage(
    NodeUUID &senderUUID,
    TrustLineUUID &trustLineUUID) :

    RoutingTableOutgoingMessage(
        senderUUID,
        trustLineUUID) {}

const Message::MessageType SecondLevelRoutingTableOutgoingMessage::typeID() const {

    return Message::MessageTypeID::SecondLevelRoutingTableOutgoingMessageType;
}
