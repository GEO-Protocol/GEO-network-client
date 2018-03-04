#include "MaxFlowCalculationTargetSndLevelMessage.h"


MaxFlowCalculationTargetSndLevelMessage::MaxFlowCalculationTargetSndLevelMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const NodeUUID &targetUUID) :

    MaxFlowCalculationMessage(
        equivalent,
        senderUUID,
        targetUUID)
{}

MaxFlowCalculationTargetSndLevelMessage::MaxFlowCalculationTargetSndLevelMessage(
    BytesShared buffer):

    MaxFlowCalculationMessage(buffer)
{}

const Message::MessageType MaxFlowCalculationTargetSndLevelMessage::typeID() const
{
    return Message::MessageType::MaxFlow_CalculationTargetSecondLevel;
}
