#include "MaxFlowCalculationTargetFstLevelMessage.h"

MaxFlowCalculationTargetFstLevelMessage::MaxFlowCalculationTargetFstLevelMessage(
    const NodeUUID& senderUUID,
    const NodeUUID& targetUUID) :

    MaxFlowCalculationMessage(senderUUID, targetUUID) {};

MaxFlowCalculationTargetFstLevelMessage::MaxFlowCalculationTargetFstLevelMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType MaxFlowCalculationTargetFstLevelMessage::typeID() const {

    return Message::MessageTypeID::MaxFlowCalculationTargetFstLevelMessageType;
}
