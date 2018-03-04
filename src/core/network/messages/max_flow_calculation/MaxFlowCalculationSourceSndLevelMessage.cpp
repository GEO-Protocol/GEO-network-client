#include "MaxFlowCalculationSourceSndLevelMessage.h"

MaxFlowCalculationSourceSndLevelMessage::MaxFlowCalculationSourceSndLevelMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID& senderUUID,
    const NodeUUID& targetUUID) :

    MaxFlowCalculationMessage(
        equivalent,
        senderUUID,
        targetUUID)
{}

MaxFlowCalculationSourceSndLevelMessage::MaxFlowCalculationSourceSndLevelMessage(
    BytesShared buffer):

    MaxFlowCalculationMessage(buffer)
{}

const Message::MessageType MaxFlowCalculationSourceSndLevelMessage::typeID() const
{
    return Message::MessageType::MaxFlow_CalculationSourceSecondLevel;
}
