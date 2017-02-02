#include "SecondLevelRoutingTableIncomingMessage.h"

SecondLevelRoutingTableIncomingMessage::SecondLevelRoutingTableIncomingMessage(
    byte *buffer) :

    RoutingTableIncomingMessage(buffer) {

}

const Message::MessageTypeID SecondLevelRoutingTableIncomingMessage::typeID() const {

    return Message::MessageTypeID::SecondLevelRoutingTableIncomingMessageType;
}
