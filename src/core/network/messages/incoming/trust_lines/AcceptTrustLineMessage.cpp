#include "AcceptTrustLineMessage.h"

AcceptTrustLineMessage::AcceptTrustLineMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType AcceptTrustLineMessage::typeID() const {

    return Message::MessageTypeID::AcceptTrustLineMessageType;
}

const TrustLineAmount &AcceptTrustLineMessage::amount() const {

    return mTrustLineAmount;
}

pair<BytesShared, size_t> AcceptTrustLineMessage::serializeToBytes() {

    auto parentBytesAndCount = TrustLinesMessage::serializeToBytes();
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

void AcceptTrustLineMessage::deserializeFromBytes(
    BytesShared buffer) {

    TrustLinesMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = TrustLinesMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    vector<byte> amountBytes(
        buffer.get() + bytesBufferOffset,
        buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
    mTrustLineAmount = bytesToTrustLineAmount(amountBytes);
}

const size_t AcceptTrustLineMessage::kRequestedBufferSize() {

    static const size_t size = TrustLinesMessage::kOffsetToInheritedBytes() + kTrustLineAmountBytesCount;
    return size;
}

MessageResult::SharedConst AcceptTrustLineMessage::resultAccepted() const {

    return MessageResult::SharedConst(
        new MessageResult(
            mSenderUUID,
            mTransactionUUID,
            kResultCodeAccepted)
    );
}

MessageResult::SharedConst AcceptTrustLineMessage::resultConflict() const {

    return MessageResult::SharedConst(
        new MessageResult(
            mSenderUUID,
            mTransactionUUID,
            kResultCodeConflict)
    );
}

MessageResult::SharedConst AcceptTrustLineMessage::resultTransactionConflict() const {

    return MessageResult::SharedConst(
        new MessageResult(
            mSenderUUID,
            mTransactionUUID,
            kResultCodeTransactionConflict)
    );
}
