#include "RequestCycleMessage.h"

RequestCycleMessage::RequestCycleMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const TrustLineAmount &amount) :

    TransactionMessage(
        equivalent,
        senderUUID,
        transactionUUID),
    mAmount(amount)
{}

RequestCycleMessage::RequestCycleMessage(
    BytesShared buffer):

    TransactionMessage(buffer)
{
    auto parentMessageOffset = TransactionMessage::kOffsetToInheritedBytes();
    auto bytesBufferOffset = buffer.get() + parentMessageOffset;
    auto amountEndOffset = bytesBufferOffset + kTrustLineBalanceBytesCount; // TODO: deserialize only non-zero
    vector<byte> amountBytes(
        bytesBufferOffset,
        amountEndOffset);

    mAmount = bytesToTrustLineAmount(amountBytes);
}

const TrustLineAmount &RequestCycleMessage::amount() const
{
    return mAmount;
}

/*!
 *
 * Throws bad_alloc;
 */
pair<BytesShared, size_t> RequestCycleMessage::serializeToBytes() const
    throw(bad_alloc)
{
    auto serializedAmount = trustLineAmountToBytes(mAmount); // TODO: serialize only non-zero
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    size_t bytesCount =
        + parentBytesAndCount.second
        + kTrustLineAmountBytesCount;

    BytesShared buffer = tryMalloc(bytesCount);
    auto initialOffset = buffer.get();
    memcpy(
        initialOffset,
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);

    auto bytesBufferOffset = initialOffset + parentBytesAndCount.second;

    memcpy(
        bytesBufferOffset,
        serializedAmount.data(),
        kTrustLineAmountBytesCount);

    return make_pair(
        buffer,
        bytesCount);
}

const size_t RequestCycleMessage::kOffsetToInheritedBytes() const
    noexcept
{
    static const size_t offset =
        TransactionMessage::kOffsetToInheritedBytes()
        + kTrustLineAmountBytesCount;

    return offset;
}
