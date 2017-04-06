#include "TransactionMessage.h"

TransactionMessage::TransactionMessage() {}

TransactionMessage::TransactionMessage(
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID) :

    SenderMessage(senderUUID),
    mTransactionUUID(transactionUUID) {}

TransactionMessage::TransactionMessage(
        BytesShared bufer)
{
    deserializeFromBytes(bufer);
}

const bool TransactionMessage::isTransactionMessage() const {

    return true;
}

const TransactionUUID &TransactionMessage::transactionUUID() const {

    return mTransactionUUID;
}

pair<BytesShared, size_t> TransactionMessage::serializeToBytes() {

    auto parentBytesAndCount = SenderMessage::serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second
                        + TransactionUUID::kBytesSize;

    BytesShared dataBytesShared = tryMalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second
    );
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        mTransactionUUID.data,
        TransactionUUID::kBytesSize
    );
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount
    );
}

void TransactionMessage::deserializeFromBytes(
    BytesShared buffer) {

    SenderMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = SenderMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    memcpy(
        mTransactionUUID.data,
        buffer.get() + bytesBufferOffset,
        TransactionUUID::kBytesSize
    );
}

const size_t TransactionMessage::kOffsetToInheritedBytes() {

    const size_t offset = SenderMessage::kOffsetToInheritedBytes()
                                 + TransactionUUID::kBytesSize;

    return offset;
}
