#include "MaxFlowCalculationSourceFstLevelOutMessage.h"

MaxFlowCalculationSourceFstLevelOutMessage::MaxFlowCalculationSourceFstLevelOutMessage(
    const NodeUUID& senderUUID,
    const NodeUUID& targetUUID) :

    MaxFlowCalculationMessage(senderUUID, targetUUID) {};

const Message::MessageType MaxFlowCalculationSourceFstLevelOutMessage::typeID() const {

    return Message::MessageTypeID::MaxFlowCalculationSourceFstLevelOutMessageType;
}
