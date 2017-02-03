#include "Response.h"

Response::Response(
    byte *buffer) {

    deserialize(buffer);
}

Response::Response(NodeUUID sender,
                   TransactionUUID transactionUUID,
                   uint16_t code) :

    TrustLinesMessage(
        sender,
        transactionUUID
    ) {

    mCode = code;
}

uint16_t Response::code() {

    return mCode;
}

pair<ConstBytesShared, size_t> Response::serialize() {

    auto parentBytesAndCount = serializeParentToBytes();

    size_t dataSize = parentBytesAndCount.second + sizeof(uint16_t);

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
    memcpy(
        data + parentBytesAndCount.second,
        &mCode,
        sizeof(uint16_t)
    );
    //----------------------------
    return make_pair(
        ConstBytesShared(data, free),
        dataSize
    );
}

void Response::deserialize(byte *buffer) {

    deserializeParentFromBytes(buffer);
    //------------------------------
    uint16_t *code = new (buffer + kOffsetToInheritBytes()) uint16_t;
    mCode = *code;
}

const Message::MessageTypeID Response::typeID() const {

    return Message::MessageTypeID::ResponseMessageType;
}
