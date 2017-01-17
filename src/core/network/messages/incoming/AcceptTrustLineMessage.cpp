#include "AcceptTrustLineMessage.h"

AcceptTrustLineMessage::AcceptTrustLineMessage(
    byte *buffer) {

    deserialize(buffer);
}

AcceptTrustLineMessage::AcceptTrustLineMessage(
    NodeUUID sender,
    TransactionUUID transactionUUID,
    uint16_t journalCode) :

    Message(sender, transactionUUID) {

    mJournalCode = journalCode;
}

pair<ConstBytesShared, size_t> AcceptTrustLineMessage::serialize() {

    size_t dataSize = sizeof(uint16_t) +
        NodeUUID::kUUIDSize +
        TransactionUUID::kUUIDSize +
        sizeof(uint16_t);
    byte * data = (byte *) malloc(dataSize);
    memset(
        data,
        0,
        dataSize
    );

    uint16_t type = typeID();
    memcpy(
        data,
        &type,
        sizeof(uint16_t)
    );

    memcpy(
        data + sizeof(uint16_t),
        mSender.data,
        NodeUUID::kUUIDSize
    );

    memcpy(
        data + sizeof(uint16_t) + NodeUUID::kUUIDSize,
        mTransactionUUID.data,
        TransactionUUID::kUUIDSize
    );

    memcpy(
        data + sizeof(uint16_t) + NodeUUID::kUUIDSize + TransactionUUID::kUUIDSize,
        &mJournalCode,
        sizeof(uint16_t)
    );

    return make_pair(
        ConstBytesShared(data, free),
        dataSize
    );

}

void AcceptTrustLineMessage::deserialize(
    byte *buffer) {

    //Message::deserialize(buffer);
    //------------------------------
    memcpy(
        mTransactionUUID.data,
        buffer,
        TransactionUUID::kUUIDSize
    );
    //------------------------------
    vector<byte> amountBytes(
        buffer + TransactionUUID::kUUIDSize,
        buffer + TransactionUUID::kUUIDSize + kTrustLineAmountSize);


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

TrustLineAmount AcceptTrustLineMessage::amount() const {

    return mTrustLineAmount;
}

const MessageResult *AcceptTrustLineMessage::resultConflict() const {

    return new MessageResult(
        sender(),
        409
    );
}


