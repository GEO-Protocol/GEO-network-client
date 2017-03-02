#include "MaxFlowCalculationTargetFstLevelInMessage.h"

MaxFlowCalculationTargetFstLevelInMessage::MaxFlowCalculationTargetFstLevelInMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType MaxFlowCalculationTargetFstLevelInMessage::typeID() const {

    return Message::MessageTypeID::MaxFlowCalculationTargetFstLevelInMessageType;
}
