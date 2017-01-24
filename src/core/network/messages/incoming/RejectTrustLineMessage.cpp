#include "RejectTrustLineMessage.h"

RejectTrustLineMessage::RejectTrustLineMessage(
    byte *buffer) {

    deserialize(buffer);
}

pair<ConstBytesShared, size_t> RejectTrustLineMessage::serialize() {

    throw NotImplementedError("RejectTrustLineMessage::serialize: "
                                  "Method not implemented.");
}

void RejectTrustLineMessage::deserialize(
    byte *buffer) {

    //------------------------------
    memcpy(
        mSenderUUID.data,
        buffer,
        NodeUUID::kBytesSize
    );
    //------------------------------
    memcpy(
        mTransactionUUID.data,
        buffer + NodeUUID::kBytesSize,
        TransactionUUID::kBytesSize
    );
    //------------------------------
    memcpy(
        mContractorUUID.data,
        buffer + NodeUUID::kBytesSize + TransactionUUID::kBytesSize,
        NodeUUID::kBytesSize
    );
}

const Message::MessageTypeID RejectTrustLineMessage::typeID() const {

    return Message::MessageTypeID::RejectTrustLineMessageType;
}

const NodeUUID &RejectTrustLineMessage::contractorUUID() const {

    return mContractorUUID;
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
