#include "MaxFlowCalculationSourceFstLevelInMessage.h"

MaxFlowCalculationSourceFstLevelInMessage::MaxFlowCalculationSourceFstLevelInMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType MaxFlowCalculationSourceFstLevelInMessage::typeID() const {

    return Message::MessageTypeID::MaxFlowCalculationSourceFstLevelInMessageType;
}