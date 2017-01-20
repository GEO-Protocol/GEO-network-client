#include "AcceptTrustLineMessage.h"

AcceptTrustLineMessage::AcceptTrustLineMessage(
    byte *buffer) {

    deserialize(buffer);
}

pair<ConstBytesShared, size_t> AcceptTrustLineMessage::serialize() {

    throw NotImplementedError("AcceptTrustLineMessage::serialize: "
                                  "Method not implemented.");
}

void AcceptTrustLineMessage::deserialize(
    byte *buffer) {
    //------------------------------
    memcpy(
        mSenderUUID.data,
        buffer,
        NodeUUID::kUUIDSize
    );
    //------------------------------
    memcpy(
        mTransactionUUID.data,
        buffer + NodeUUID::kUUIDSize,
        TransactionUUID::kUUIDSize
    );
    //------------------------------
    vector<byte> amountBytes(
        buffer + NodeUUID::kUUIDSize + TransactionUUID::kUUIDSize,
        buffer + NodeUUID::kUUIDSize + TransactionUUID::kUUIDSize + kTrustLineAmountSize);

    vector<byte> amountNotZeroBytes;
    amountNotZeroBytes.reserve(kTrustLineAmountSize);

    for (auto &item : amountBytes) {
        if (item != 0) {
            amountNotZeroBytes.push_back(item);
        }
    }

    if (amountNotZeroBytes.size() > 0) {
        import_bits(
            mTrustLineAmount,
            amountNotZeroBytes.begin(),
            amountNotZeroBytes.end()
        );

    } else {
        import_bits(
            mTrustLineAmount,
            amountBytes.begin(),
            amountBytes.end()
        );
    }
    //------------------------------
}

const Message::MessageTypeID AcceptTrustLineMessage::typeID() const {

    return Message::MessageTypeID::AcceptTrustLineMessageType;
}

const TrustLineAmount &AcceptTrustLineMessage::amount() const {

    return mTrustLineAmount;
}

MessageResult::Shared AcceptTrustLineMessage::resultAccepted() const {

    return MessageResult::Shared(
        new MessageResult(
            mSenderUUID,
            mTransactionUUID,
            kResultCodeAccepted)
    );
}

MessageResult::Shared AcceptTrustLineMessage::resultConflict() const {

    return MessageResult::Shared(
        new MessageResult(
            mSenderUUID,
            mTransactionUUID,
            kResultCodeConflict)
    );
}

MessageResult::Shared AcceptTrustLineMessage::resultTransactionConflict() const {

    return MessageResult::Shared(
        new MessageResult(
            mSenderUUID,
            mTransactionUUID,
            kResultCodeTransactionConflict)
    );
}

MessageResult::Shared AcceptTrustLineMessage::customCodeResult(
    uint16_t code) const {

    return MessageResult::Shared(
        new MessageResult(
            mSenderUUID,
            mTransactionUUID,
            code)
    );
}


