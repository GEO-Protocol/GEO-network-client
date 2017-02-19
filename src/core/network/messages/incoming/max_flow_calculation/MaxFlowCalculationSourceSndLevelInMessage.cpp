//
// Created by mc on 17.02.17.
//

#include "MaxFlowCalculationSourceSndLevelInMessage.h"

MaxFlowCalculationSourceSndLevelInMessage::MaxFlowCalculationSourceSndLevelInMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType MaxFlowCalculationSourceSndLevelInMessage::typeID() const {

    return Message::MessageTypeID::MaxFlowCalculationSourceSndLevelInMessageType;
}

const size_t MaxFlowCalculationSourceSndLevelInMessage::kRequestedBufferSize() {

    static const size_t size = MaxFlowCalculationMessage::kOffsetToInheritedBytes();
    return size;
}
