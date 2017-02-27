#include "RoutingTablesResponse.h"

RoutingTablesResponse::RoutingTablesResponse(
    const NodeUUID &sender,
    const uint16_t code) :

    SenderMessage(sender) {

    mCode = code;
}

RoutingTablesResponse::RoutingTablesResponse(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType RoutingTablesResponse::typeID() const {

    return Message::MessageTypeID::RoutingTablesResponseMessageType;
}

const uint16_t RoutingTablesResponse::code() const{

    return mCode;
}

pair<BytesShared, size_t> RoutingTablesResponse::serializeToBytes() {

    auto parentBytesAndCount = SenderMessage::serializeToBytes();

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
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount
    );
}

void RoutingTablesResponse::deserializeFromBytes(BytesShared buffer) {

    SenderMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = SenderMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    uint16_t *code = new (buffer.get() + bytesBufferOffset) uint16_t;
    mCode = *code;
}