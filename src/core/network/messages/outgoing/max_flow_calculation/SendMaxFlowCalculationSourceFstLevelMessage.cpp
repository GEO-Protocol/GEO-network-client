#include "SendMaxFlowCalculationSourceFstLevelMessage.h"

SendMaxFlowCalculationSourceFstLevelMessage::SendMaxFlowCalculationSourceFstLevelMessage(
    const NodeUUID& senderUUID,
    const NodeUUID& targetUUID) :

    MaxFlowCalculationMessage(senderUUID, targetUUID) {};

const Message::MessageType SendMaxFlowCalculationSourceFstLevelMessage::typeID() const {

    return Message::MessageTypeID::SendMaxFlowCalculationSourceFstLevelMessageType;
}
