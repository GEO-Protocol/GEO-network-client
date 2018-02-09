#include "MessageResult.h"

MessageResult::MessageResult(
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const uint16_t resultCode) :

    mSenderUUID(senderUUID),
    mTransactionUUID(transactionUUID),
    mTimestampCompleted(utc_now())
{
    mResultCode = resultCode;
}

const NodeUUID &MessageResult::senderUUID() const
{
    return mSenderUUID;
}

const TransactionUUID MessageResult::transactionUUID() const
{
    return mTransactionUUID;
}

const uint16_t MessageResult::resultCode() const
{
    return mResultCode;
}

const DateTime &MessageResult::timestampCompleted() const
{
    return mTimestampCompleted;
}

const string MessageResult::serialize() const
{
    return mSenderUUID.stringUUID() + kTokensSeparator +
           to_string(mResultCode) + kCommandsSeparator;
}
