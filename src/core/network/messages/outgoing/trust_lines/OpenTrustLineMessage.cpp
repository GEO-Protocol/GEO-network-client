#include "OpenTrustLineMessage.h"

OpenTrustLineMessage::OpenTrustLineMessage(
    NodeUUID &sender,
    TransactionUUID &transactionUUID,
    TrustLineAmount amount) :

    TrustLinesMessage(
        sender,
        transactionUUID
    ),
    mTrustLineAmount(amount) {}

const Message::MessageTypeID OpenTrustLineMessage::typeID() const {

    return Message::MessageTypeID::OpenTrustLineMessageType;
}

pair<ConstBytesShared, size_t> OpenTrustLineMessage::serialize() {

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
        mTrustLineAmount,
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

void OpenTrustLineMessage::deserialize(
    byte* buffer) {

    throw NotImplementedError("OpenTrustLineMessage::deserialize: "
                                  "Method not implemented.");
}