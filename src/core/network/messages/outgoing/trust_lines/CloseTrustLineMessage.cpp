#include "CloseTrustLineMessage.h"

CloseTrustLineMessage::CloseTrustLineMessage(
    const NodeUUID &sender,
    const TransactionUUID &transactionUUID,
    const NodeUUID &contractorUUID) :

    TrustLinesMessage(
        sender,
        transactionUUID
    ),
    mContractorUUID(contractorUUID){

}

const Message::MessageType CloseTrustLineMessage::typeID() const {

    return Message::MessageTypeID::CloseTrustLineMessageType;
}

pair<BytesShared, size_t> CloseTrustLineMessage::serializeToBytes() {

    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second
                        + NodeUUID::kBytesSize;

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
    //----------------------------
    return make_pair(
        dataBytesShared,
        bytesCount
    );
}

void CloseTrustLineMessage::deserializeFromBytes(
    BytesShared buffer) {

    throw NotImplementedError("CloseTrustLineMessage::deserializeFromBytes: "
                                  "Method not implemented.");
}