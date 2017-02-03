#include "CloseTrustLineMessage.h"

CloseTrustLineMessage::CloseTrustLineMessage(
    NodeUUID &sender,
    TransactionUUID &transactionUUID,
    NodeUUID &contractorUUID) :

    TrustLinesMessage(
        sender,
        transactionUUID
    ),
    mContractorUUID(contractorUUID){

}

const Message::MessageTypeID CloseTrustLineMessage::typeID() const {

    return Message::MessageTypeID::CloseTrustLineMessageType;
}

pair<ConstBytesShared, size_t> CloseTrustLineMessage::serialize() {

    auto parentBytesAndCount = serializeParentToBytes();

    size_t dataSize = parentBytesAndCount.second + NodeUUID::kBytesSize;

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