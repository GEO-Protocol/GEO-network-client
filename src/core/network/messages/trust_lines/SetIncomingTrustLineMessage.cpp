#include "SetIncomingTrustLineMessage.h"


SetIncomingTrustLineMessage::SetIncomingTrustLineMessage(
    const NodeUUID &sender,
    const TransactionUUID &transactionUUID,
    const TrustLineAmount &amount)
    noexcept :

    TransactionMessage(
        sender,
        transactionUUID),
    mAmount(amount)
{}

SetIncomingTrustLineMessage::SetIncomingTrustLineMessage(
    BytesShared buffer)
    noexcept :

    TransactionMessage(buffer)
{
    // todo: use desrializer

    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
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

    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second
                        + kTrustLineAmountBytesCount;

    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second
    );
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    vector<byte> buffer = trustLineAmountToBytes(mAmount);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        buffer.data(),
        buffer.size()
    );
    //----------------------------
    return make_pair(
        dataBytesShared,
        bytesCount
    );
}
