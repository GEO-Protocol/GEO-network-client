#include "MaxFlowCalculationMessage.h"

MaxFlowCalculationMessage::MaxFlowCalculationMessage() {}

MaxFlowCalculationMessage::MaxFlowCalculationMessage(
    const NodeUUID &targetUUID) :

    Message(),

    mTargetUUID(targetUUID){}

const NodeUUID &MaxFlowCalculationMessage::targetUUID() const {

    return mTargetUUID;
}

pair<BytesShared, size_t> MaxFlowCalculationMessage::serializeToBytes() {

    size_t bytesCount = sizeof(MessageType) + NodeUUID::kBytesSize;

    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    MessageType type = typeID();
    memcpy(
        dataBytesShared.get(),
        &type,
        sizeof(MessageType));
    dataBytesOffset += sizeof(MessageType);
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        mTargetUUID.data,
        NodeUUID::kBytesSize);
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

void MaxFlowCalculationMessage::deserializeFromBytes(
    BytesShared buffer) {

    size_t bytesBufferOffset = 0;
    MessageType *messageType = new (buffer.get()) MessageType;
    bytesBufferOffset += sizeof(MessageType);
    //----------------------------------------------------
    memcpy(
        mTargetUUID.data,
        buffer.get() + bytesBufferOffset,
        NodeUUID::kBytesSize);
}

const size_t MaxFlowCalculationMessage::kOffsetToInheritedBytes() {

    static const size_t offset = sizeof(MessageType) + NodeUUID::kBytesSize;
    return offset;
}
