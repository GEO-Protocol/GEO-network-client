#include "SetIncomingTrustLineMessage.h"


SetIncomingTrustLineMessage::SetIncomingTrustLineMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &sender,
    const TransactionUUID &transactionUUID,
    const NodeUUID &destinationUUID,
    const KeyNumber keyNumber,
    const lamport::Signature::Shared signature,
    const TrustLineAmount &amount)
    noexcept :

    AuditMessage(
        equivalent,
        sender,
        transactionUUID,
        destinationUUID,
        keyNumber,
        signature),
    mAmount(amount)
{}

SetIncomingTrustLineMessage::SetIncomingTrustLineMessage(
    BytesShared buffer)
    noexcept :
    AuditMessage(buffer)
{
    // todo: use desrializer

    size_t bytesBufferOffset = AuditMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    vector<byte> amountBytes(
        buffer.get() + bytesBufferOffset,
        buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
    mAmount = bytesToTrustLineAmount(amountBytes);
}


const Message::MessageType SetIncomingTrustLineMessage::typeID() const
    noexcept
{
    return Message::TrustLines_SetIncoming;
}

const TrustLineAmount &SetIncomingTrustLineMessage::amount() const
    noexcept
{
    return mAmount;
}

pair<BytesShared, size_t> SetIncomingTrustLineMessage::serializeToBytes() const
{
    // todo: use serializer

    auto parentBytesAndCount = AuditMessage::serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second
                        + kTrustLineAmountBytesCount;

    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    vector<byte> buffer = trustLineAmountToBytes(mAmount);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        buffer.data(),
        buffer.size());
    //----------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

const size_t SetIncomingTrustLineMessage::kOffsetToInheritedBytes() const
    noexcept
{
    static const auto kOffset =
            AuditMessage::kOffsetToInheritedBytes()
            + kTrustLineAmountBytesCount;

    return kOffset;
}

