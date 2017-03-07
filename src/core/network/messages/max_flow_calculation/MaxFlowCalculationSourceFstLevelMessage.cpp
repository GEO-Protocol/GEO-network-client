#include "MaxFlowCalculationSourceFstLevelMessage.h"

MaxFlowCalculationSourceFstLevelMessage::MaxFlowCalculationSourceFstLevelMessage(
        const NodeUUID& senderUUID,
        const NodeUUID& targetUUID) :

        MaxFlowCalculationMessage(senderUUID, targetUUID) {};

MaxFlowCalculationSourceFstLevelMessage::MaxFlowCalculationSourceFstLevelMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType MaxFlowCalculationSourceFstLevelMessage::typeID() const {

    return Message::MessageTypeID::MaxFlowCalculationSourceFstLevelMessageType;
}