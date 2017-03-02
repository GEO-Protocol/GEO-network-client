#include "ReceiveMaxFlowCalculationOnTargetMessage.h"

ReceiveMaxFlowCalculationOnTargetMessage::ReceiveMaxFlowCalculationOnTargetMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType ReceiveMaxFlowCalculationOnTargetMessage::typeID() const {

    return Message::MessageTypeID::ReceiveMaxFlowCalculationOnTargetMessageType;
}
