//
// Created by mc on 16.02.17.
//

#include "SendMaxFlowCalculationTargetFstLevelMessage.h"

SendMaxFlowCalculationTargetFstLevelMessage::SendMaxFlowCalculationTargetFstLevelMessage(
    const NodeUUID& senderUUID,
    const NodeUUID& targetUUID) :

    MaxFlowCalculationMessage(senderUUID, targetUUID) {};

const Message::MessageType SendMaxFlowCalculationTargetFstLevelMessage::typeID() const {

    return Message::MessageTypeID::SendMaxFlowCalculationTargetFstLevelMessageType;
}