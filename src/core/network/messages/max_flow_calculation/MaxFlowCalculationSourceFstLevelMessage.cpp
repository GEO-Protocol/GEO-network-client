#include "MaxFlowCalculationSourceFstLevelMessage.h"

MaxFlowCalculationSourceFstLevelMessage::MaxFlowCalculationSourceFstLevelMessage(
    const NodeUUID& senderUUID) :

    SenderMessage(senderUUID) {};

MaxFlowCalculationSourceFstLevelMessage::MaxFlowCalculationSourceFstLevelMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType MaxFlowCalculationSourceFstLevelMessage::typeID() const {

    return Message::MessageTypeID::MaxFlowCalculationSourceFstLevelMessageType;
}