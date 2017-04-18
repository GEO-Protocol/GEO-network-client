#include "AcceptTrustLineMessage.h"


AcceptTrustLineMessage::AcceptTrustLineMessage(
    BytesShared buffer):

    BaseTrustLineMessage(buffer)
{
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    vector<byte> amountBytes(
        buffer.get() + bytesBufferOffset,
        buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
    mTrustLineAmount = bytesToTrustLineAmount(amountBytes);
}

const Message::MessageType AcceptTrustLineMessage::typeID() const
{
    return Message::MessageType::TrustLines_Accept;
}

const TrustLineAmount &AcceptTrustLineMessage::amount() const
{
    return mTrustLineAmount;
}

pair<BytesShared, size_t> AcceptTrustLineMessage::serializeToBytes() const
    throw(bad_alloc)
{

    auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second +
                        kTrustLineAmountBytesCount;

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

MessageResult::SharedConst AcceptTrustLineMessage::resultAccepted() const {
    // todo: refactor
    return MessageResult::SharedConst(
        new MessageResult(
            senderUUID,
            mTransactionUUID,
            kResultCodeAccepted)
    );
}

MessageResult::SharedConst AcceptTrustLineMessage::resultConflict() const {

    // todo: refactor
    return MessageResult::SharedConst(
        new MessageResult(
            senderUUID,
            mTransactionUUID,
            kResultCodeConflict)
    );
}

MessageResult::SharedConst AcceptTrustLineMessage::resultTransactionConflict() const {

    // todo: refactor
    return MessageResult::SharedConst(
        new MessageResult(
            senderUUID,
            mTransactionUUID,
            kResultCodeTransactionConflict)
    );
}