#include "ReserveBalanceRequestMessage.h"


ReserveBalanceRequestMessage::ReserveBalanceRequestMessage(
    const NodeUUID& senderUUID,
    const TransactionUUID& transactionUUID,
    const TrustLineAmount& amount,
    const NodeUUID& nextNodeInThePath) :

    TransactionMessage(
        senderUUID,
        transactionUUID),
    mAmount(amount),
    mNextNodeInPathUUID(nextNodeInThePath)
{
}

ReserveBalanceRequestMessage::ReserveBalanceRequestMessage(
    BytesShared buffer)
{
    deserializeFromBytes(buffer);
}

const TrustLineAmount&ReserveBalanceRequestMessage::amount() const
{
    return mAmount;
}

const NodeUUID&ReserveBalanceRequestMessage::nextNodeInThePathUUID() const
{
    return mNextNodeInPathUUID;
}

const Message::MessageType ReserveBalanceRequestMessage::typeID() const
{
    return Message::Payments_ReserveBalanceRequest;
}

/**
 * @throws bad_alloc;
 */
pair<BytesShared, size_t> ReserveBalanceRequestMessage::serializeToBytes()
{
    auto serializedAmount =
        trustLineAmountToBytes(mAmount); // TODO: serialize only non-zero

    auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    size_t bytesCount =
        + parentBytesAndCount.second
        + serializedAmount.size()
        + NodeUUID::kBytesSize;

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
        serializedAmount.size());

    auto uuidOffset = amountOffset + serializedAmount.size();
    memcpy(
        uuidOffset,
        mNextNodeInPathUUID.data,
        mNextNodeInPathUUID.kBytesSize);

    return make_pair(
        buffer,
        bytesCount);
}

void ReserveBalanceRequestMessage::deserializeFromBytes(
    BytesShared buffer)
{
    TransactionMessage::deserializeFromBytes(buffer);

    auto parentMessageOffset = TransactionMessage::kOffsetToInheritedBytes();
    auto amountOffset = buffer.get() + parentMessageOffset;
    auto amountEndOffset = amountOffset + kTrustLineBalanceBytesCount; // TODO: deserialize only non-zero
    vector<byte> amountBytes(
        amountOffset,
        amountEndOffset);

    mAmount = bytesToTrustLineAmount(amountBytes);

    memcpy(
        mNextNodeInPathUUID.data,
        amountEndOffset + NodeUUID::kBytesSize,
        NodeUUID::kBytesSize);
}
