#include "SendMaxFlowCalculationTargetFstLevelMessage.h"

SendMaxFlowCalculationTargetFstLevelMessage::SendMaxFlowCalculationTargetFstLevelMessage(
    const NodeUUID& senderUUID,
    const NodeUUID& targetUUID) :

    MaxFlowCalculationMessage(senderUUID, targetUUID) {};

const Message::MessageType SendMaxFlowCalculationTargetFstLevelMessage::typeID() const {

    return Message::MessageTypeID::SendMaxFlowCalculationTargetFstLevelMessageType;
}