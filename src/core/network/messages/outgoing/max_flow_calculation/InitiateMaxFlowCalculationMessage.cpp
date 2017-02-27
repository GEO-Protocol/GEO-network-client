#include "InitiateMaxFlowCalculationMessage.h"

InitiateMaxFlowCalculationMessage::InitiateMaxFlowCalculationMessage(
    const NodeUUID& senderUUID,
    const NodeUUID& targetUUID) :

    MaxFlowCalculationMessage(senderUUID, targetUUID) {};

const Message::MessageType InitiateMaxFlowCalculationMessage::typeID() const {

    return Message::MessageTypeID::InitiateMaxFlowCalculationMessageType;
}
