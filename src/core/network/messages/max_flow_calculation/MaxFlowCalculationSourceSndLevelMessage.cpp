#include "MaxFlowCalculationSourceSndLevelMessage.h"

MaxFlowCalculationSourceSndLevelMessage::MaxFlowCalculationSourceSndLevelMessage(
    const NodeUUID& senderUUID,
    const NodeUUID& targetUUID) :

    MaxFlowCalculationMessage(senderUUID, targetUUID) {};

MaxFlowCalculationSourceSndLevelMessage::MaxFlowCalculationSourceSndLevelMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType MaxFlowCalculationSourceSndLevelMessage::typeID() const {

    return Message::MessageTypeID::MaxFlowCalculationSourceSndLevelMessageType;
}
