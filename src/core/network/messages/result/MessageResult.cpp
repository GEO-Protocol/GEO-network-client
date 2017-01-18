#include "MessageResult.h"

MessageResult::MessageResult(
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const uint16_t resultCode) :

    mSenderUUID(senderUUID),
    mTransactionUUID(transactionUUID),
    mTimestampCompleted(boost::posix_time::microsec_clock::universal_time()) {

    mResultCode = resultCode;
}

const NodeUUID &MessageResult::senderUUID() const {

    return mSenderUUID;
}

const TransactionUUID MessageResult::transactionUUID() const {

    return mTransactionUUID;
}

const uint16_t MessageResult::resultCode() const {

    return mResultCode;
}

const Timestamp &MessageResult::timestampCompleted() const {

    return mTimestampCompleted;
}

const string MessageResult::serialize() const {

    return mSenderUUID.stringUUID() + "\t" +
           boost::lexical_cast<string>(mResultCode) + "\n";
}
