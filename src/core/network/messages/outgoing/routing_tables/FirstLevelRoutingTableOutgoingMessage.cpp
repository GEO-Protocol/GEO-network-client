#include "FirstLevelRoutingTableOutgoingMessage.h"

FirstLevelRoutingTableOutgoingMessage::FirstLevelRoutingTableOutgoingMessage(
    NodeUUID &sender,
    TransactionUUID &transactionUUID,
    NodeUUID &contractor) :

    RoutingTableOutgoingMessage(
        sender,
        transactionUUID,
        contractor) {}


const Message::MessageTypeID FirstLevelRoutingTableOutgoingMessage::typeID() const {

    return Message::MessageTypeID::FirstLevelRoutingTableOutgoingMessageType;
}