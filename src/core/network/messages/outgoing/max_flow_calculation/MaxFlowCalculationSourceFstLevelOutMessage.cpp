//
// Created by mc on 17.02.17.
//

#include "MaxFlowCalculationSourceFstLevelOutMessage.h"

MaxFlowCalculationSourceFstLevelOutMessage::MaxFlowCalculationSourceFstLevelOutMessage(
    NodeUUID &targetUUID) :

    MaxFlowCalculationMessage(targetUUID) {};

const Message::MessageType MaxFlowCalculationSourceFstLevelOutMessage::typeID() const {

    return Message::MessageTypeID::MaxFlowCalculationSourceFstLevelOutMessageType;
}
