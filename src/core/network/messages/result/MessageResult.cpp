#include "MessageResult.h"

MessageResult::MessageResult(
    const NodeUUID &senderUUID,
    const uint16_t resultCode) :

    mSenderUUID(senderUUID),
    mTimestampCompleted(boost::posix_time::microsec_clock::universal_time()) {

    mResultCode = resultCode;
}

const NodeUUID &MessageResult::commandUUID() const {

    return mSenderUUID;
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
