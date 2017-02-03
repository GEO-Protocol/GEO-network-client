#include "UpdateTrustLineMessage.h"

UpdateTrustLineMessage::UpdateTrustLineMessage(
    byte *buffer) {

    deserialize(buffer);
}

const Message::MessageTypeID UpdateTrustLineMessage::typeID() const {

    return Message::MessageTypeID::UpdateTrustLineMessageType;
}

const TrustLineAmount &UpdateTrustLineMessage::newAmount() const {

    return mNewTrustLineAmount;
}

pair<ConstBytesShared, size_t> UpdateTrustLineMessage::serialize() {

    auto parentBytesAndCount = serializeParentToBytes();

    size_t dataSize = parentBytesAndCount.second + kTrustLineAmountSize;

    byte *data = (byte *) calloc(
        dataSize,
        sizeof(byte)
    );

    memcpy(
        data,
        const_cast<byte *> (parentBytesAndCount.first.get()),
        parentBytesAndCount.second
    );
    //----------------------------
    vector<byte> trustAmountBytesBuffer;
    trustAmountBytesBuffer.reserve(kTrustLineAmountSize);
    export_bits(
        mNewTrustLineAmount,
        back_inserter(trustAmountBytesBuffer),
        8
    );
    size_t unusedBufferPlace = kTrustLineAmountSize - trustAmountBytesBuffer.size();
    for (size_t i = 0; i < unusedBufferPlace; ++i) {
        trustAmountBytesBuffer.push_back(0);
    }
    memcpy(
        data + parentBytesAndCount.second,
        trustAmountBytesBuffer.data(),
        trustAmountBytesBuffer.size()
    );
    //----------------------------
    return make_pair(
        ConstBytesShared(data, free),
        dataSize
    );
}

void UpdateTrustLineMessage::deserialize(
    byte *buffer) {

    deserializeParentFromBytes(buffer);
    //------------------------------
    vector<byte> amountBytes(
        buffer + kOffsetToInheritBytes(),
        buffer + kOffsetToInheritBytes() + kTrustLineAmountSize);

    vector<byte> amountNotZeroBytes;
    amountNotZeroBytes.reserve(kTrustLineAmountSize);

    for (auto &amountByte : amountBytes) {
        if (amountByte != 0) {
            amountNotZeroBytes.push_back(amountByte);
        }
    }

    if (amountNotZeroBytes.size() > 0) {
        import_bits(
            mNewTrustLineAmount,
            amountNotZeroBytes.begin(),
            amountNotZeroBytes.end()
        );

    } else {
        import_bits(
            mNewTrustLineAmount,
            amountBytes.begin(),
            amountBytes.end()
        );
    }
}

const size_t UpdateTrustLineMessage::kRequestedBufferSize() {

    const size_t trustAmountBytesSize = 32;
    static const size_t size = kOffsetToInheritBytes() + trustAmountBytesSize;
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

MessageResult::Shared UpdateTrustLineMessage::resulConflict() const {

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