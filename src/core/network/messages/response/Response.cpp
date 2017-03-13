﻿#include "Response.h"

Response::Response(const NodeUUID &sender,
                   const TransactionUUID &transactionUUID,
                   const uint16_t code) :

    TransactionMessage(
        sender,
        transactionUUID
    ) {

    mCode = code;
}

Response::Response(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const bool Response::isTransactionMessage() const {

    return true;
}

const Message::MessageType Response::typeID() const {

    return Message::MessageTypeID::ResponseMessageType;
}

uint16_t Response::code() {

    return mCode;
}

pair<BytesShared, size_t> Response::serializeToBytes() {

    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

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

    TransactionMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //------------------------------
    uint16_t *code = new (buffer.get() + bytesBufferOffset) uint16_t;
    mCode = *code;
}
