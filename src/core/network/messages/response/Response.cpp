#include "Response.h"

Response::Response(
    const NodeUUID &sender,
    const TransactionUUID &transactionUUID,
    const SerializedResponseCode code) :

    TransactionMessage(
        sender,
        transactionUUID)
{
    mCode = code;
}

Response::Response(
    BytesShared buffer):

    TransactionMessage(buffer)
{
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //------------------------------
    SerializedResponseCode *code = new (buffer.get() + bytesBufferOffset) SerializedResponseCode;
    mCode = *code;
}

uint16_t Response::code()
{
    return mCode;
}

pair<BytesShared, size_t> Response::serializeToBytes() const
    throw(bad_alloc)
{
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second
                        + sizeof(SerializedResponseCode);

    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mCode,
        sizeof(SerializedResponseCode));
    //----------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

void Response::deserializeFromBytes(
    BytesShared buffer)
{
    TransactionMessage::deserializeFromBytes(buffer);
}
