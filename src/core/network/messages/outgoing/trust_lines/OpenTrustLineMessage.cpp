#include "OpenTrustLineMessage.h"

OpenTrustLineMessage::OpenTrustLineMessage(
    NodeUUID &sender,
    TransactionUUID &transactionUUID,
    TrustLineAmount &amount) :

    TrustLinesMessage(
        sender,
        transactionUUID
    ),
    mTrustLineAmount(amount) {}

const Message::MessageType OpenTrustLineMessage::typeID() const {

    return Message::MessageTypeID::OpenTrustLineMessageType;
}

pair<BytesShared, size_t> OpenTrustLineMessage::serializeToBytes() {

    auto parentBytesAndCount = TrustLinesMessage::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second +
        kTrustLineAmountBytesCount;
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second
    );
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    vector<byte> buffer = trustLineAmountToBytes(mTrustLineAmount);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        buffer.data(),
        buffer.size()
    );
    //----------------------------
    return make_pair(
        dataBytesShared,
        bytesCount
    );
}

void OpenTrustLineMessage::deserializeFromBytes(
    BytesShared buffer) {

    throw NotImplementedError("OpenTrustLineMessage::deserializeFromBytes: "
                                  "Method not implemented.");
}