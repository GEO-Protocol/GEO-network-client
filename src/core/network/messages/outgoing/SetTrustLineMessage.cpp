#include "SetTrustLineMessage.h"

SetTrustLineMessage::SetTrustLineMessage(
    NodeUUID &sender,
    TransactionUUID &transactionUUID,
    TrustLineAmount newAmount) {

}

pair<ConstBytesShared, size_t> SetTrustLineMessage::serialize() {

    size_t dataSize = sizeof(uint16_t) +
                      NodeUUID::kBytesSize +
                      TransactionUUID::kBytesSize +
                      kTrustLineAmountSize;
    byte *data = (byte *) malloc (dataSize);
    memset(
        data,
        0,
        dataSize
    );

    //----------------------------
    uint16_t type = typeID();
    memcpy(
        data,
        &type,
        sizeof(uint16_t)
    );
    //----------------------------
    memcpy(
        data + sizeof(uint16_t),
        mSenderUUID.data,
        NodeUUID::kBytesSize
    );
    //----------------------------
    memcpy(
        data + sizeof(uint16_t) + NodeUUID::kBytesSize,
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
        data + sizeof(uint16_t) + NodeUUID::kBytesSize + TransactionUUID::kBytesSize,
        buffer.data(),
        buffer.size()
    );
    //----------------------------

    return make_pair(
        ConstBytesShared(data, free),
        dataSize
    );
}

void SetTrustLineMessage::deserialize(
    byte *buffer) {

    throw NotImplementedError("SetTrustLineMessage::deserialize: "
                                  "Method not implemented.");
}

const Message::MessageTypeID SetTrustLineMessage::typeID() const {

    return Message::MessageTypeID::SetTrustLineMessageType;
}


