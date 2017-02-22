#include "FirstLevelRoutingTableOutgoingMessage.h"

FirstLevelRoutingTableOutgoingMessage::FirstLevelRoutingTableOutgoingMessage(
    NodeUUID &senderUUID,
    TrustLineUUID &trustLineUUID) :

    RoutingTableOutgoingMessage(
        senderUUID,
        trustLineUUID) {}


const Message::MessageType FirstLevelRoutingTableOutgoingMessage::typeID() const {

    return Message::MessageTypeID::FirstLevelRoutingTableOutgoingMessageType;
}
