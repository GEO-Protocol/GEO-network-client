#include "FirstLevelRoutingTableOutgoingMessage.h"

FirstLevelRoutingTableOutgoingMessage::FirstLevelRoutingTableOutgoingMessage(
    NodeUUID &senderUUID,
    NodeUUID &contractorUUID,
    TrustLineUUID &trustLineUUID) :

    RoutingTableOutgoingMessage(
        senderUUID,
        contractorUUID,
        trustLineUUID) {}


const Message::MessageTypeID FirstLevelRoutingTableOutgoingMessage::typeID() const {

    return Message::MessageTypeID::FirstLevelRoutingTableOutgoingMessageType;
}