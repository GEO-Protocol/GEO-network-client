#include "MaxFlowCalculationSourceFstLevelInMessage.h"

MaxFlowCalculationSourceFstLevelInMessage::MaxFlowCalculationSourceFstLevelInMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType MaxFlowCalculationSourceFstLevelInMessage::typeID() const {

    return Message::MessageTypeID::MaxFlowCalculationSourceFstLevelInMessageType;
}

const size_t MaxFlowCalculationSourceFstLevelInMessage::kRequestedBufferSize() {

    static const size_t size = MaxFlowCalculationMessage::kOffsetToInheritedBytes();
    return size;
}