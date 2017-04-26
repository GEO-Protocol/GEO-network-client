#include "MaxFlowCalculationSourceSndLevelMessage.h"

MaxFlowCalculationSourceSndLevelMessage::MaxFlowCalculationSourceSndLevelMessage(
    const NodeUUID& senderUUID,
    const NodeUUID& targetUUID) :

    MaxFlowCalculationMessage(senderUUID, targetUUID)
{}

MaxFlowCalculationSourceSndLevelMessage::MaxFlowCalculationSourceSndLevelMessage(
    BytesShared buffer):

    MaxFlowCalculationMessage(buffer)
{}

const Message::MessageType MaxFlowCalculationSourceSndLevelMessage::typeID() const {

    return Message::MessageType::MaxFlow_CalculationSourceSecondLevel;
}
