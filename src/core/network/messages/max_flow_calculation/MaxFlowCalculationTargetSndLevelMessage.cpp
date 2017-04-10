#include "MaxFlowCalculationTargetSndLevelMessage.h"

MaxFlowCalculationTargetSndLevelMessage::MaxFlowCalculationTargetSndLevelMessage(
    const NodeUUID& senderUUID,
    const NodeUUID& targetUUID) :

    MaxFlowCalculationMessage(senderUUID, targetUUID) {};

MaxFlowCalculationTargetSndLevelMessage::MaxFlowCalculationTargetSndLevelMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType MaxFlowCalculationTargetSndLevelMessage::typeID() const {

    return Message::MessageTypeID::MaxFlowCalculationTargetSndLevelMessageType;
}
