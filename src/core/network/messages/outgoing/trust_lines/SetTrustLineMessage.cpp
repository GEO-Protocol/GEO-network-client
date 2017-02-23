#include "SetTrustLineMessage.h"

SetTrustLineMessage::SetTrustLineMessage(
    const NodeUUID &sender,
    const TransactionUUID &transactionUUID,
    const TrustLineAmount &newAmount) :

    TrustLinesMessage(
        sender,
        transactionUUID
    ),
    mNewTrustLineAmount(newAmount) {}

const Message::MessageType SetTrustLineMessage::typeID() const {

    return Message::MessageTypeID::SetTrustLineMessageType;
}

pair<BytesShared, size_t> SetTrustLineMessage::serializeToBytes() {

    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second
                        + kTrustLineAmountBytesCount;

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
    vector<byte> buffer = trustLineAmountToBytes(mNewTrustLineAmount);
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

void SetTrustLineMessage::deserializeFromBytes(
    BytesShared buffer) {

    throw NotImplementedError("SetTrustLineMessage::deserializeFromBytes: "
                                  "Method not implemented.");
}