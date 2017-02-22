//
// Created by mc on 16.02.17.
//

#include "MaxFlowCalculationTargetFstLevelInMessage.h"

MaxFlowCalculationTargetFstLevelInMessage::MaxFlowCalculationTargetFstLevelInMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType MaxFlowCalculationTargetFstLevelInMessage::typeID() const {

    return Message::MessageTypeID::MaxFlowCalculationTargetFstLevelInMessageType;
}

const size_t MaxFlowCalculationTargetFstLevelInMessage::kRequestedBufferSize() {

    static const size_t size = MaxFlowCalculationMessage::kOffsetToInheritedBytes();
    return size;
}
