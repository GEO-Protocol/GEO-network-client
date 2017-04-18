#include "OpenTrustLineMessage.h"


OpenTrustLineMessage::OpenTrustLineMessage(
    const NodeUUID &sender,
    const TransactionUUID &transactionUUID,
    const TrustLineAmount &amount)
    noexcept :

    TransactionMessage(
        sender,
        transactionUUID),
    mTrustLineAmount(amount)
{}

const Message::MessageType OpenTrustLineMessage::typeID() const
    noexcept
{
    return Message::MessageType::TrustLines_Open;
}

/*
 * ToDo: rewrite me with bytes deserializer
 */
pair<BytesShared, size_t> OpenTrustLineMessage::serializeToBytes() const
    throw (bad_alloc)
{
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
    vector<byte> buffer = trustLineAmountToBytes(mTrustLineAmount);
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