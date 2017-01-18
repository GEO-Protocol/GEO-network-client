#include "OpenTrustLineMessage.h"

OpenTrustLineMessage::OpenTrustLineMessage(
    NodeUUID sender,
    TransactionUUID transactionUUID,
    TrustLineAmount amount) :

    Message(sender, transactionUUID),
    mTrustLineAmount(amount) {}

pair<ConstBytesShared, size_t> OpenTrustLineMessage::serialize() {

    size_t dataSize = sizeof(uint16_t) +
        NodeUUID::kUUIDSize +
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
      mSenderUUID.data,
      NodeUUID::kUUIDSize
    );

    memcpy(
        data + sizeof(uint16_t) + NodeUUID::kUUIDSize,
        mTransactionUUID.data,
        TransactionUUID::kUUIDSize
    );

    vector<byte> buffer;
    buffer.reserve(kTrustLineAmountSize);
    export_bits(mTrustLineAmount, back_inserter(buffer), 8);
    size_t unusedBufferPlace = kTrustLineAmountSize - buffer.size();
    for (size_t i = 0; i < unusedBufferPlace; ++i) {
        buffer.push_back(0);
    }
    memcpy(
        data + sizeof(uint16_t) + NodeUUID::kUUIDSize + TransactionUUID::kUUIDSize,
        buffer.data(),
        buffer.size()
    );

    return make_pair(
        ConstBytesShared(data, free),
        dataSize
    );
}

void OpenTrustLineMessage::deserialize(
    byte* buffer) {

    throw NotImplementedError("OpenTrustLineMessage::deserialize: "
                                  "Method not implemented.");
}

const Message::MessageTypeID OpenTrustLineMessage::typeID() const {
    return Message::MessageTypeID::OpenTrustLineMessageType;
}





