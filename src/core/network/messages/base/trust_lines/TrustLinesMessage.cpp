#include "TrustLinesMessage.h"

TrustLinesMessage::TrustLinesMessage() {}

TrustLinesMessage::TrustLinesMessage(
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID) :

    TransactionMessage(
        senderUUID,
        transactionUUID
    ) {}

const TransactionUUID &TrustLinesMessage::transactionUUID() const {

    return mTransactionUUID;
}

MessageResult::SharedConst TrustLinesMessage::customCodeResult(
    const uint16_t code) const {

    return MessageResult::SharedConst(
        new MessageResult(
            mSenderUUID,
            mTransactionUUID,
            code)
    );
}