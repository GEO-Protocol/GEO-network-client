#include "CloseTrustLineMessage.h"

CloseTrustLineMessage::CloseTrustLineMessage(
    NodeUUID &sender,
    TransactionUUID &transactionUUID,
    NodeUUID &contractorUUID) {

}

pair<ConstBytesShared, size_t> CloseTrustLineMessage::serialize() {

    size_t dataSize = sizeof(uint16_t) +
                      NodeUUID::kUUIDSize +
                      TransactionUUID::kUUIDSize +
                      NodeUUID::kUUIDSize;
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
        NodeUUID::kUUIDSize
    );
    //----------------------------
    memcpy(
        data + sizeof(uint16_t) + NodeUUID::kUUIDSize,
        mTransactionUUID.data,
        TransactionUUID::kUUIDSize
    );
    //----------------------------
    memcpy(
        data + sizeof(uint16_t) + NodeUUID::kUUIDSize + TransactionUUID::kUUIDSize,
        mContractorUUID.data,
        TransactionUUID::kUUIDSize
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
