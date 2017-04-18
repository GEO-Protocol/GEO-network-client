#include "UpdateTrustLineMessage.h"


UpdateTrustLineMessage::UpdateTrustLineMessage(
    BytesShared buffer)
    noexcept :

    TransactionMessage(buffer)
{
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    vector<byte> amountBytes(
        buffer.get() + bytesBufferOffset,
        buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
    mNewTrustLineAmount = bytesToTrustLineAmount(amountBytes);
}

const Message::MessageType UpdateTrustLineMessage::typeID() const
    noexcept
{
    return Message::MessageType::TrustLines_Update;
}

const TrustLineAmount &UpdateTrustLineMessage::newAmount() const
    noexcept
{
    return mNewTrustLineAmount;
}

// todo: rewrite me with bytes serializer
pair<BytesShared, size_t> UpdateTrustLineMessage::serializeToBytes() const
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

MessageResult::SharedConst UpdateTrustLineMessage::resultAccepted() const
    noexcept
{

    return MessageResult::SharedConst(
        new MessageResult(
            senderUUID,
            mTransactionUUID,
            kResultCodeAccepted)
    );
}

MessageResult::SharedConst UpdateTrustLineMessage::resultRejected() const
    noexcept
{
    return MessageResult::SharedConst(
        new MessageResult(
            senderUUID,
            mTransactionUUID,
            kResultCodeRejected)
    );
}

MessageResult::SharedConst UpdateTrustLineMessage::resultConflict() const
    noexcept
{
    return MessageResult::SharedConst(
        new MessageResult(
            senderUUID,
            mTransactionUUID,
            kResultCodeTrustLineAbsent)
    );
}