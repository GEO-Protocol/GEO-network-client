#include "RequestMessage.h"


RequestMessage::RequestMessage(
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const TrustLineAmount &amount) :

    TransactionMessage(
        senderUUID,
        transactionUUID),
    mAmount(amount)
{}

RequestMessage::RequestMessage(
    BytesShared buffer):

    TransactionMessage(buffer)
{
    deserializeFromBytes(buffer);
}

const TrustLineAmount &RequestMessage::amount() const
{
    return mAmount;
}

/*!
 *
 * Throws bad_alloc;
 */
pair<BytesShared, size_t> RequestMessage::serializeToBytes() {

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

    auto amountOffset = initialOffset + parentBytesAndCount.second;
    memcpy(
        amountOffset,
        serializedAmount.data(),
        kTrustLineAmountBytesCount);

    return make_pair(
        buffer,
                bytesCount);
}

const size_t RequestMessage::kOffsetToInheritedBytes() const
    noexcept
{
    static const size_t offset =
        TransactionMessage::kOffsetToInheritedBytes()
        + kTrustLineAmountBytesCount;

    return offset;
}

void RequestMessage::deserializeFromBytes(
    BytesShared buffer)
{
    auto parentMessageOffset = TransactionMessage::kOffsetToInheritedBytes();
    auto amountOffset = buffer.get() + parentMessageOffset;
    auto amountEndOffset = amountOffset + kTrustLineBalanceBytesCount; // TODO: deserialize only non-zero
    vector<byte> amountBytes(
        amountOffset,
        amountEndOffset);

    mAmount = bytesToTrustLineAmount(amountBytes);
}
