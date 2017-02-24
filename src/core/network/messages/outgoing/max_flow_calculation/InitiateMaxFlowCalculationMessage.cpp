//
// Created by mc on 14.02.17.
//

#include "InitiateMaxFlowCalculationMessage.h"

InitiateMaxFlowCalculationMessage::InitiateMaxFlowCalculationMessage(
        NodeUUID &targetUUID) :

    MaxFlowCalculationMessage(targetUUID) {};

const Message::MessageType InitiateMaxFlowCalculationMessage::typeID() const {

    return Message::MessageTypeID::InitiateMaxFlowCalculationMessageType;
}
