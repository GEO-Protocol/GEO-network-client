#include "SetTrustLineMessage.h"

SetTrustLineMessage::SetTrustLineMessage(
    NodeUUID &sender,
    TransactionUUID &transactionUUID,
    TrustLineAmount newAmount) :

    TrustLinesMessage(
        sender,
        transactionUUID
    ),
    mNewTrustLineAmount(newAmount) {}

const Message::MessageTypeID SetTrustLineMessage::typeID() const {

    return Message::MessageTypeID::SetTrustLineMessageType;
}

pair<ConstBytesShared, size_t> SetTrustLineMessage::serialize() {

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

void SetTrustLineMessage::deserialize(
    byte *buffer) {

    throw NotImplementedError("SetTrustLineMessage::deserialize: "
                                  "Method not implemented.");
}