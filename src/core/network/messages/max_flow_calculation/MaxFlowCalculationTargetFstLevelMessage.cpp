#include "MaxFlowCalculationTargetFstLevelMessage.h"

MaxFlowCalculationTargetFstLevelMessage::MaxFlowCalculationTargetFstLevelMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID& senderUUID,
    const NodeUUID& targetUUID) :

    MaxFlowCalculationMessage(
        equivalent,
        senderUUID,
        targetUUID)
{}

MaxFlowCalculationTargetFstLevelMessage::MaxFlowCalculationTargetFstLevelMessage(
    BytesShared buffer):

    MaxFlowCalculationMessage(buffer)
{}

const Message::MessageType MaxFlowCalculationTargetFstLevelMessage::typeID() const
{
    return Message::MessageType::MaxFlow_CalculationTargetFirstLevel;
}
