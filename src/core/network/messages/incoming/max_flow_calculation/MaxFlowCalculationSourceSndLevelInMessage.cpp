#include "MaxFlowCalculationSourceSndLevelInMessage.h"

MaxFlowCalculationSourceSndLevelInMessage::MaxFlowCalculationSourceSndLevelInMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType MaxFlowCalculationSourceSndLevelInMessage::typeID() const {

    return Message::MessageTypeID::MaxFlowCalculationSourceSndLevelInMessageType;
}
