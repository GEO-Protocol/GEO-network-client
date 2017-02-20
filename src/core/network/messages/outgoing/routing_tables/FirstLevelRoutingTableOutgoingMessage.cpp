#include "FirstLevelRoutingTableOutgoingMessage.h"

FirstLevelRoutingTableOutgoingMessage::FirstLevelRoutingTableOutgoingMessage(
    const NodeUUID &senderUUID) :

    RoutingTableOutgoingMessage(
        senderUUID) {}


const Message::MessageType FirstLevelRoutingTableOutgoingMessage::typeID() const {

    return Message::MessageTypeID::FirstLevelRoutingTableOutgoingMessageType;
}