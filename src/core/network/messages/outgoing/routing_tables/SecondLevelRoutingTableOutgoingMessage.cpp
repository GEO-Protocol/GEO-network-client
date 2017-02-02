#include "SecondLevelRoutingTableOutgoingMessage.h"

SecondLevelRoutingTableOutgoingMessage::SecondLevelRoutingTableOutgoingMessage(
    NodeUUID &sender,
    TransactionUUID &transactionUUID,
    NodeUUID &contractor) :

    RoutingTableOutgoingMessage(
        sender,
        transactionUUID,
        contractor) {}

const Message::MessageTypeID SecondLevelRoutingTableOutgoingMessage::typeID() const {

    return Message::MessageTypeID::SecondLevelRoutingTableOutgoingMessageType;
}
