#include "RejectTrustLineMessage.h"

RejectTrustLineMessage::RejectTrustLineMessage(
    NodeUUID sender,
    TransactionUUID transactionUUID,
    uint16_t resultCode) :

    Message(sender, transactionUUID) {

    mResultCode = resultCode;
}

pair<ConstBytesShared, size_t> RejectTrustLineMessage::serialize() {

}

void RejectTrustLineMessage::deserialize(byte *buffer) {

    throw NotImplementedError("RejectTrustLineMessage::deserialize: "
                                  "Method not implemented.");
}

const Message::MessageTypeID RejectTrustLineMessage::typeID() const {
    return OpenTrustLineMessageType;
}

uint16_t RejectTrustLineMessage::resultCode() const {

    return mResultCode;
}
