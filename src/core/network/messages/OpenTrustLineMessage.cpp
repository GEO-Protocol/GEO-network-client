#include "OpenTrustLineMessage.h"

OpenTrustLineMessage::OpenTrustLineMessage(
    TransactionUUID &transactionUUID,
    TrustLineAmount &amount) :

    mTransactionUUID(transactionUUID),
    mTrustLineAmount(amount) {}

pair<ConstBytesShared, size_t> OpenTrustLineMessage::serialize() const {

    size_t dataSize = sizeof(uint16_t) +
        TransactionUUID::kUUIDSize +
        kTrustLineAmountSize;
    byte *data = (byte *) malloc (dataSize);
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
        mTransactionUUID.data,
        TransactionUUID::kUUIDSize
    );

    vector<byte> *buffer = new vector<byte>;
    export_bits(mTrustLineAmount, back_inserter(*buffer), 8);
    for (size_t i = 0; i < kTrustLineAmountSize - buffer->size(); ++i) {
        buffer->push_back(0);
    }
    memcpy(
        data + sizeof(uint16_t) + TransactionUUID::kUUIDSize,
        buffer->data(),
        kTrustLineAmountSize
    );
    delete buffer;

    return make_pair(
        ConstBytesShared(data, free),
        dataSize);
}

const Message::MessageTypeID OpenTrustLineMessage::typeID() const {
    return Message::MessageTypeID::OpenTrustLineMessageType;
}




