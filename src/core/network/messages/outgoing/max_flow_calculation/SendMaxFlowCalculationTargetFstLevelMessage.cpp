//
// Created by mc on 16.02.17.
//

#include "SendMaxFlowCalculationTargetFstLevelMessage.h"

SendMaxFlowCalculationTargetFstLevelMessage::SendMaxFlowCalculationTargetFstLevelMessage(
    NodeUUID &targetUUID) :

    MaxFlowCalculationMessage(targetUUID) {};

const Message::MessageType SendMaxFlowCalculationTargetFstLevelMessage::typeID() const {

    return Message::MessageTypeID::SendMaxFlowCalculationTargetFstLevelMessageType;
}