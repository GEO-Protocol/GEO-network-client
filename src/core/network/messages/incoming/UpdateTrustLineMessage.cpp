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

    size_t dataSize = NodeUUID::kBytesSize +
                      TransactionUUID::kBytesSize +
                      kTrustLineAmountSize;
    byte *data = (byte *) calloc (dataSize, sizeof(byte));
    //----------------------------
    memcpy(
        data,
        mSenderUUID.data,
        NodeUUID::kBytesSize
    );
    //----------------------------
    memcpy(
        data + NodeUUID::kBytesSize,
        mTransactionUUID.data,
        TransactionUUID::kBytesSize
    );
    //----------------------------
    vector<byte> buffer;
    buffer.reserve(kTrustLineAmountSize);
    export_bits(
        mNewTrustLineAmount,
        back_inserter(buffer),
        8
    );
    size_t unusedBufferPlace = kTrustLineAmountSize - buffer.size();
    for (size_t i = 0; i < unusedBufferPlace; ++i) {
        buffer.push_back(0);
    }
    memcpy(
        data + NodeUUID::kBytesSize + TransactionUUID::kBytesSize,
        buffer.data(),
        buffer.size()
    );
    //----------------------------

    return make_pair(
        ConstBytesShared(data, free),
        dataSize
    );
}

void UpdateTrustLineMessage::deserialize(
    byte *buffer) {
    //------------------------------
    memcpy(
        mSenderUUID.data,
        buffer,
        NodeUUID::kBytesSize
    );
    //------------------------------
    memcpy(
        mTransactionUUID.data,
        buffer + NodeUUID::kBytesSize,
        TransactionUUID::kBytesSize
    );
    //------------------------------
    vector<byte> amountBytes(
        buffer + NodeUUID::kBytesSize + TransactionUUID::kBytesSize,
        buffer + NodeUUID::kBytesSize + TransactionUUID::kBytesSize + kTrustLineAmountSize);

    vector<byte> amountNotZeroBytes;
    amountNotZeroBytes.reserve(kTrustLineAmountSize);

    for (auto &item : amountBytes) {
        if (item != 0) {
            amountNotZeroBytes.push_back(item);
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
    static const size_t size = NodeUUID::kBytesSize + TransactionUUID::kBytesSize + trustAmountBytesSize;
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