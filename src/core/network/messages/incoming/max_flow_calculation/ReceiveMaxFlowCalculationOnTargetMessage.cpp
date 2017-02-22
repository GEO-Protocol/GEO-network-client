//
// Created by mc on 15.02.17.
//

#include "ReceiveMaxFlowCalculationOnTargetMessage.h"

ReceiveMaxFlowCalculationOnTargetMessage::ReceiveMaxFlowCalculationOnTargetMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType ReceiveMaxFlowCalculationOnTargetMessage::typeID() const {

    return Message::MessageTypeID::ReceiveMaxFlowCalculationOnTargetMessageType;
}

const size_t ReceiveMaxFlowCalculationOnTargetMessage::kRequestedBufferSize() {

    static const size_t size = MaxFlowCalculationMessage::kOffsetToInheritedBytes();
    return size;
}
