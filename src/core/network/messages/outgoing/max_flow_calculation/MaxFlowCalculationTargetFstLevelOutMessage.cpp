//
// Created by mc on 17.02.17.
//

#include "MaxFlowCalculationTargetFstLevelOutMessage.h"

MaxFlowCalculationTargetFstLevelOutMessage::MaxFlowCalculationTargetFstLevelOutMessage(
    NodeUUID &targetUUID) :

    MaxFlowCalculationMessage(targetUUID) {};

const Message::MessageType MaxFlowCalculationTargetFstLevelOutMessage::typeID() const {

    return Message::MessageTypeID::MaxFlowCalculationTargetFstLevelOutMessageType;
}
