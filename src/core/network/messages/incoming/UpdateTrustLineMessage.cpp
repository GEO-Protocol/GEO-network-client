#include "UpdateTrustLineMessage.h"

UpdateTrustLineMessage::UpdateTrustLineMessage(
    byte *buffer) {

    deserialize(buffer);
}

pair<ConstBytesShared, size_t> UpdateTrustLineMessage::serialize() {

    throw NotImplementedError("UpdateTrustLineMessage::serialize: "
                                  "Method not implemented.");
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
    //------------------------------
}

const Message::MessageTypeID UpdateTrustLineMessage::typeID() const {

    return Message::MessageTypeID::UpdateTrustLineMessageType;
}

const TrustLineAmount &UpdateTrustLineMessage::newAmount() const {

    return mNewTrustLineAmount;
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
