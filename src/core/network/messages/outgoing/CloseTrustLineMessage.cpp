#include "CloseTrustLineMessage.h"

CloseTrustLineMessage::CloseTrustLineMessage(
    NodeUUID &sender,
    TransactionUUID &transactionUUID,
    NodeUUID &contractorUUID) :

    Message(
        sender,
        transactionUUID
    ),
    mContractorUUID(contractorUUID){

}

pair<ConstBytesShared, size_t> CloseTrustLineMessage::serialize() {

    size_t dataSize = sizeof(uint16_t) +
                      NodeUUID::kBytesSize +
                      TransactionUUID::kBytesSize +
                      NodeUUID::kBytesSize;
    byte *data = (byte *) malloc (dataSize);
    memset(
        data,
        0,
        dataSize
    );

    //----------------------------
    uint16_t type = typeID();
    memcpy(
        data,
        &type,
        sizeof(uint16_t)
    );
    //----------------------------
    memcpy(
        data + sizeof(uint16_t),
        mSenderUUID.data,
        NodeUUID::kBytesSize
    );
    //----------------------------
    memcpy(
        data + sizeof(uint16_t) + NodeUUID::kBytesSize,
        mTransactionUUID.data,
        TransactionUUID::kBytesSize
    );
    //----------------------------
    memcpy(
        data + sizeof(uint16_t) + NodeUUID::kBytesSize + TransactionUUID::kBytesSize,
        mContractorUUID.data,
        NodeUUID::kBytesSize
    );
    //----------------------------

    return make_pair(
        ConstBytesShared(data, free),
        dataSize
    );
}

void CloseTrustLineMessage::deserialize(
    byte *buffer) {

    throw NotImplementedError("CloseTrustLineMessage::deserialize: "
                                  "Method not implemented.");
}

const Message::MessageTypeID CloseTrustLineMessage::typeID() const {

    return Message::MessageTypeID::CloseTrustLineMessageType;
}
