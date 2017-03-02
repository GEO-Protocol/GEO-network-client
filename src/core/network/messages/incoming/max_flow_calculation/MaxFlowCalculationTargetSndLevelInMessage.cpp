#include "MaxFlowCalculationTargetSndLevelInMessage.h"

MaxFlowCalculationTargetSndLevelInMessage::MaxFlowCalculationTargetSndLevelInMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType MaxFlowCalculationTargetSndLevelInMessage::typeID() const {

    return Message::MessageTypeID::MaxFlowCalculationTargetSndLevelInMessageType;
}
