#include "FirstLevelRoutingTableIncomingMessage.h"

FirstLevelRoutingTableIncomingMessage::FirstLevelRoutingTableIncomingMessage(
    BytesShared buffer) :

    RoutingTableIncomingMessage(buffer) {}

const Message::MessageType FirstLevelRoutingTableIncomingMessage::typeID() const {

    return Message::MessageTypeID::FirstLevelRoutingTableIncomingMessageType;
}