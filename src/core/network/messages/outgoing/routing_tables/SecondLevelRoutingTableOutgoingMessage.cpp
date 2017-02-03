#include "SecondLevelRoutingTableOutgoingMessage.h"

SecondLevelRoutingTableOutgoingMessage::SecondLevelRoutingTableOutgoingMessage(
    NodeUUID &senderUUID,
    NodeUUID &contractorUUID,
    TrustLineUUID &trustLineUUID) :

    RoutingTableOutgoingMessage(
        senderUUID,
        contractorUUID,
        trustLineUUID) {}

const Message::MessageTypeID SecondLevelRoutingTableOutgoingMessage::typeID() const {

    return Message::MessageTypeID::SecondLevelRoutingTableOutgoingMessageType;
}
