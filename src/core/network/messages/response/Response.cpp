#include "Response.h"

Response::Response(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

Response::Response(NodeUUID &sender,
                   TransactionUUID &transactionUUID,
                   uint16_t code) :

    TrustLinesMessage(
        sender,
        transactionUUID
    ) {

    mCode = code;
}

const Message::MessageType Response::typeID() const {

    return Message::MessageTypeID::ResponseMessageType;
}

uint16_t Response::code() {

    return mCode;
}

pair<BytesShared, size_t> Response::serializeToBytes() {

    auto parentBytesAndCount = TrustLinesMessage::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second
                        + sizeof(uint16_t);
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
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mCode,
        sizeof(uint16_t)
    );
    //----------------------------
    return make_pair(
        dataBytesShared,
        bytesCount
    );
}

void Response::deserializeFromBytes(
    BytesShared buffer) {

    TrustLinesMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = TrustLinesMessage::kOffsetToInheritedBytes();
    //------------------------------
    uint16_t *code = new (buffer.get() + bytesBufferOffset) uint16_t;
    mCode = *code;
}
