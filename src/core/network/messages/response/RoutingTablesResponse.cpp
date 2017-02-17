#include "RoutingTablesResponse.h"

RoutingTablesResponse::RoutingTablesResponse(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

RoutingTablesResponse::RoutingTablesResponse(
    NodeUUID &sender,
    NodeUUID &contractor,
    uint16_t code) :

    Message(sender),
    mContractorUUID(contractor) {

    mCode = code;
}

const Message::MessageType RoutingTablesResponse::typeID() const {

    return Message::MessageTypeID::RoutingTablesResponseMessageType;
}

const NodeUUID &RoutingTablesResponse::contractorUUID() const {

    return mContractorUUID;
}

const TransactionUUID &RoutingTablesResponse::transactionUUID() const {

    throw ConflictError("RoutingTablesResponse::transactionUUID: "
                            "Method not implemented");
}

const uint16_t RoutingTablesResponse::code() const{

    return mCode;
}

pair<BytesShared, size_t> RoutingTablesResponse::serializeToBytes() {

    auto parentBytesAndCount = Message::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second
                        + NodeUUID::kBytesSize
                        + sizeof(uint16_t);
    ;
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
        mContractorUUID.data,
        NodeUUID::kBytesSize
    );
    dataBytesOffset += NodeUUID::kBytesSize;
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

    Message::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = Message::kOffsetToInheritedBytes();
    //----------------------------------------------------
    memcpy(
      mContractorUUID.data,
      buffer.get() + bytesBufferOffset,
      NodeUUID::kBytesSize
    );
    bytesBufferOffset += NodeUUID::kBytesSize;
    //----------------------------------------------------
    uint16_t *code = new (buffer.get() + bytesBufferOffset) uint16_t;
    mCode = *code;
}