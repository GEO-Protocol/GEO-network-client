#include "UpdateTrustLineMessage.h"

UpdateTrustLineMessage::UpdateTrustLineMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType UpdateTrustLineMessage::typeID() const {

    return Message::MessageTypeID::AcceptTrustLineMessageType;
}

const TrustLineAmount &UpdateTrustLineMessage::newAmount() const {

    return mNewTrustLineAmount;
}

pair<BytesShared, size_t> UpdateTrustLineMessage::serializeToBytes() {

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

void UpdateTrustLineMessage::deserializeFromBytes(
    BytesShared buffer) {

    TrustLinesMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = TrustLinesMessage::inheritED();
    //----------------------------------------------------
    vector<byte> amountBytes(
        buffer.get() + bytesBufferOffset,
        buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
    mNewTrustLineAmount = bytesToTrustLineAmount(amountBytes);
}

const size_t UpdateTrustLineMessage::kRequestedBufferSize() {

    static const size_t size = TrustLinesMessage::inheritED() + kTrustLineAmountBytesCount;
    return size;
}

MessageResult::Shared UpdateTrustLineMessage::resultAccepted() const {

    return MessageResult::Shared(
        new MessageResult(
            mSenderUUID,
            mTransactionUUID,
            kResultCodeAccepted)
    );
}

MessageResult::Shared UpdateTrustLineMessage::resultRejected() const {

    return MessageResult::Shared(
        new MessageResult(
            mSenderUUID,
            mTransactionUUID,
            kResultCodeRejected)
    );
}

MessageResult::Shared UpdateTrustLineMessage::resultConflict() const {

    return MessageResult::Shared(
        new MessageResult(
            mSenderUUID,
            mTransactionUUID,
            kResultCodeConflict)
    );
}

MessageResult::Shared UpdateTrustLineMessage::resultTransactionConflict() const {

    return MessageResult::Shared(
        new MessageResult(
            mSenderUUID,
            mTransactionUUID,
            kResultCodeTransactionConflict)
    );
}

MessageResult::Shared UpdateTrustLineMessage::customCodeResult(uint16_t code) const {

    return MessageResult::Shared(
        new MessageResult(
            mSenderUUID,
            mTransactionUUID,
            code)
    );
}