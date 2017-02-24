//
// Created by mc on 16.02.17.
//

#include "SendMaxFlowCalculationSourceFstLevelMessage.h"

SendMaxFlowCalculationSourceFstLevelMessage::SendMaxFlowCalculationSourceFstLevelMessage(
    NodeUUID &targetUUID) :

    MaxFlowCalculationMessage(targetUUID) {};

const Message::MessageType SendMaxFlowCalculationSourceFstLevelMessage::typeID() const {

    return Message::MessageTypeID::SendMaxFlowCalculationSourceFstLevelMessageType;
}
