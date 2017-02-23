#include "SecondLevelRoutingTableIncomingMessage.h"

SecondLevelRoutingTableIncomingMessage::SecondLevelRoutingTableIncomingMessage(
    BytesShared buffer) :

    RoutingTableIncomingMessage(buffer) {

}

const Message::MessageType SecondLevelRoutingTableIncomingMessage::typeID() const {

    return Message::MessageTypeID::SecondLevelRoutingTableIncomingMessageType;
}
