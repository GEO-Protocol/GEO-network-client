#include "RejectTrustLineMessage.h"

RejectTrustLineMessage::RejectTrustLineMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType RejectTrustLineMessage::typeID() const {

    return Message::MessageTypeID::RejectTrustLineMessageType;
}

const NodeUUID &RejectTrustLineMessage::contractorUUID() const {

    return mContractorUUID;
}

pair<BytesShared, size_t> RejectTrustLineMessage::serializeToBytes() {

    auto parentBytesAndCount = TrustLinesMessage::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second +
                        NodeUUID::kBytesSize;
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

void RejectTrustLineMessage::deserializeFromBytes(
    BytesShared buffer) {

    TrustLinesMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = TrustLinesMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    memcpy(
        mContractorUUID.data,
        buffer.get() + bytesBufferOffset,
        NodeUUID::kBytesSize
    );
}

const size_t RejectTrustLineMessage::kRequestedBufferSize() {

    static const size_t size = TrustLinesMessage::kOffsetToInheritedBytes() + NodeUUID::kBytesSize;
    return size;
}

MessageResult::SharedConst RejectTrustLineMessage::resultRejected() {

    return MessageResult::SharedConst(
        new MessageResult(
            mSenderUUID,
            mTransactionUUID,
            kResultCodeRejected)
    );
}

MessageResult::SharedConst RejectTrustLineMessage::resultRejectDelayed() {

    return MessageResult::SharedConst(
        new MessageResult(
            mSenderUUID,
            mTransactionUUID,
            kResultCodeRejectDelayed)
    );
}

MessageResult::SharedConst RejectTrustLineMessage::resultTransactionConflict() const {

    return MessageResult::SharedConst(
        new MessageResult(
            mSenderUUID,
            mTransactionUUID,
            kResultCodeTransactionConflict)
    );
}
