#include "ObservingRequestCheckMessage.h"

ObservingRequestCheckMessage::ObservingRequestCheckMessage(
    const TransactionUUID &transactionUUID) :
    mTransactionUUID(transactionUUID)
{}

const Message::MessageType ObservingRequestCheckMessage::typeID() const
{
    return Message::Observing_CheckRequest;
}

pair<BytesShared, size_t> ObservingRequestCheckMessage::serializeToBytes() const
{
    const auto parentBytesAndCount = Message::serializeToBytes();

    const auto kBufferSize =
        parentBytesAndCount.second
        + TransactionUUID::kBytesSize;

    BytesShared buffer = tryMalloc(kBufferSize);

    size_t dataBytesOffset = 0;
    // Parent message content
    memcpy(
        buffer.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;

    memcpy(
        buffer.get() + dataBytesOffset,
        mTransactionUUID.data,
        TransactionUUID::kBytesSize);

    return make_pair(
        buffer,
        kBufferSize);
}