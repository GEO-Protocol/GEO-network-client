#include "MaxFlowCalculationTargetSndLevelInMessage.h"

MaxFlowCalculationTargetSndLevelInMessage::MaxFlowCalculationTargetSndLevelInMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType MaxFlowCalculationTargetSndLevelInMessage::typeID() const {

    return Message::MessageTypeID::MaxFlowCalculationTargetSndLevelInMessageType;
}

const size_t MaxFlowCalculationTargetSndLevelInMessage::kRequestedBufferSize() {

    static const size_t size = MaxFlowCalculationMessage::kOffsetToInheritedBytes();
    return size;
}
