#include "FirstLevelRoutingTableIncomingMessage.h"

FirstLevelRoutingTableIncomingMessage::FirstLevelRoutingTableIncomingMessage(
    byte *buffer) :

    RoutingTableIncomingMessage(buffer) {}

const Message::MessageTypeID FirstLevelRoutingTableIncomingMessage::typeID() const {

    return Message::MessageTypeID::FirstLevelRoutingTableIncomingMessageType;
}