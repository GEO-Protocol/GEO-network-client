#include "InitiateMaxFlowCalculationMessage.h"

InitiateMaxFlowCalculationMessage::InitiateMaxFlowCalculationMessage(
    const NodeUUID& senderUUID) :

    SenderMessage(senderUUID) {};

InitiateMaxFlowCalculationMessage::InitiateMaxFlowCalculationMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType InitiateMaxFlowCalculationMessage::typeID() const {

    return Message::MessageTypeID::InitiateMaxFlowCalculationMessageType;
}
