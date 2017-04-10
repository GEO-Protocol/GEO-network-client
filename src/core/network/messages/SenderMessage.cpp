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

    auto parentBytesAndCount = Message::serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second
                        + NodeUUID::kBytesSize;

    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //-----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second
    );
    dataBytesOffset += parentBytesAndCount.second;
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

    memcpy(
        mSenderUUID.data,
        buffer.get() + Message::kOffsetToInheritedBytes(),
        NodeUUID::kBytesSize);
}

const size_t SenderMessage::kOffsetToInheritedBytes() {

    static const size_t offset = Message::kOffsetToInheritedBytes()
                                 + NodeUUID::kBytesSize;

    return offset;
}