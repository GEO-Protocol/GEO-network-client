#include "SetTrustLineMessage.h"


SetTrustLineMessage::SetTrustLineMessage(
    const NodeUUID &sender,
    const TransactionUUID &transactionUUID,
    const TrustLineAmount &newAmount)
    noexcept :

    TransactionMessage(
        sender,
        transactionUUID),
    mNewTrustLineAmount(newAmount)
{}

const Message::MessageType SetTrustLineMessage::typeID() const
    noexcept
{
    return Message::MessageType::TrustLines_Set;
}

// todo: refactor me
pair<BytesShared, size_t> SetTrustLineMessage::serializeToBytes() const
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
    vector<byte> buffer = trustLineAmountToBytes(mNewTrustLineAmount);
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