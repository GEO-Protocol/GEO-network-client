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

const Message::MessageType CloseTrustLineMessage::typeID() const {

    return Message::MessageTypeID::SetTrustLineMessageType;
}

pair<BytesShared, size_t> CloseTrustLineMessage::serializeToBytes() {

    auto parentBytesAndCount = TrustLinesMessage::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second +
                        kTrustLineAmountBytesCount;
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

    throw NotImplementedError("OpenTrustLineMessage::deserializeFromBytes: "
                                  "Method not implemented.");
}