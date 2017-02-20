#include "SenderMessage.h"

SenderMessage::SenderMessage() {

}

SenderMessage::SenderMessage(
    const NodeUUID &senderUUID) :

    mSenderUUID(senderUUID){}

const NodeUUID &SenderMessage::senderUUID() const {

    return mSenderUUID;
}

pair<BytesShared, size_t> SenderMessage::serializeToBytes() {

    size_t bytesCount = sizeof(MessageType)
                        + NodeUUID::kBytesSize;

    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //-----------------------------------------------------
    MessageType type = typeID();
    memcpy(
        dataBytesShared.get(),
        &type,
        sizeof(MessageType)
    );
    dataBytesOffset += sizeof(MessageType);
    //-----------------------------------------------------
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        mSenderUUID.data,
        NodeUUID::kBytesSize
    );
    //-----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount
    );
}

void SenderMessage::deserializeFromBytes(
    BytesShared buffer) {

    size_t bytesBufferOffset = 0;

    MessageType *messageType = new (buffer.get()) MessageType;
    bytesBufferOffset += sizeof(MessageType);
    //-----------------------------------------------------
    memcpy(
        mSenderUUID.data,
        buffer.get() + bytesBufferOffset,
        NodeUUID::kBytesSize
    );
}

const size_t SenderMessage::kOffsetToInheritedBytes() {

    static const size_t offset = sizeof(MessageType)
                                 + NodeUUID::kBytesSize;

    return offset;
}