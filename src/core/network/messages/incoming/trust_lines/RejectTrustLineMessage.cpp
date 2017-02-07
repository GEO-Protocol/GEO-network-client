#include "RejectTrustLineMessage.h"

RejectTrustLineMessage::RejectTrustLineMessage(
    byte *buffer) {

    deserialize(buffer);
}

const Message::MessageTypeID RejectTrustLineMessage::typeID() const {

    return Message::MessageTypeID::RejectTrustLineMessageType;
}

const NodeUUID &RejectTrustLineMessage::contractorUUID() const {

    return mContractorUUID;
}

pair<ConstBytesShared, size_t> RejectTrustLineMessage::serialize() {

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

void RejectTrustLineMessage::deserialize(
    byte *buffer) {

    deserializeParentFromBytes(buffer);
    //------------------------------
    memcpy(
        mContractorUUID.data,
        buffer + kOffsetToInheritBytes(),
        NodeUUID::kBytesSize
    );
}

const size_t RejectTrustLineMessage::kRequestedBufferSize() {

    static const size_t size = kOffsetToInheritBytes() + NodeUUID::kBytesSize;
    return size;
}

MessageResult::Shared RejectTrustLineMessage::resultRejected() {

    return MessageResult::Shared(
        new MessageResult(
            mSenderUUID,
            mTransactionUUID,
            kResultCodeRejected)
    );
}

MessageResult::Shared RejectTrustLineMessage::resultRejectDelayed() {

    return MessageResult::Shared(
        new MessageResult(
            mSenderUUID,
            mTransactionUUID,
            kResultCodeRejectDelayed)
    );
}

MessageResult::Shared RejectTrustLineMessage::resultTransactionConflict() const {

    return MessageResult::Shared(
        new MessageResult(
            mSenderUUID,
            mTransactionUUID,
            kResultCodeTransactionConflict)
    );
}