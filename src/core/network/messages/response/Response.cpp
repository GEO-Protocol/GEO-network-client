#include "Response.h"

Response::Response(
    byte *buffer) {

    deserialize(buffer);
}

Response::Response(NodeUUID sender,
                   TransactionUUID transactionUUID,
                   uint16_t code) :

    Message(sender, transactionUUID) {

    mCode = code;
}

pair<ConstBytesShared, size_t> Response::serialize() {

    size_t dataSize = sizeof(uint16_t) +
                      NodeUUID::kUUIDSize +
                      TransactionUUID::kUUIDSize +
                      sizeof(uint16_t);
    byte * data = (byte *) malloc(dataSize);
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

    memcpy(
        data + sizeof(uint16_t) + NodeUUID::kUUIDSize + TransactionUUID::kUUIDSize,
        &mCode,
        sizeof(uint16_t)
    );

    return make_pair(
        ConstBytesShared(data, free),
        dataSize
    );
}

void Response::deserialize(byte *buffer) {

    //------------------------------
    memcpy(
        mSenderUUID.data,
        buffer,
        NodeUUID::kUUIDSize
    );
    //------------------------------
    memcpy(
        mTransactionUUID.data,
        buffer + NodeUUID::kUUIDSize,
        TransactionUUID::kUUIDSize
    );
    //------------------------------
    uint16_t *code = new (buffer + NodeUUID::kUUIDSize + TransactionUUID::kUUIDSize) uint16_t;
    mCode = *code;
}

const Message::MessageTypeID Response::typeID() const {

    return Message::MessageTypeID::ResponseMessageType;
}
