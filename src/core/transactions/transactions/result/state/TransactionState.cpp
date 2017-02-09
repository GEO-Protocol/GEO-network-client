#include "TransactionState.h"

TransactionState::TransactionState(
    MicrosecondsTimestamp awakeTimestamp,
    bool flushToPermanentStorage) :

    mAwakeningTimestamp(awakeTimestamp) {

    mFlushToPermanentStorage = flushToPermanentStorage;
}

TransactionState::TransactionState(
    Message::MessageTypeID requiredMessageType,
    bool flushToPermanentStorage) {

    mRequiredMessageTypes.push_back(requiredMessageType);
    mFlushToPermanentStorage = flushToPermanentStorage;
}

TransactionState::TransactionState(
    MicrosecondsTimestamp awakeTimestamp,
    Message::MessageTypeID requiredMessageType,
    bool flushToPermanentStorage) :

    mAwakeningTimestamp(awakeTimestamp) {

    mRequiredMessageTypes.push_back(requiredMessageType);
    mFlushToPermanentStorage = flushToPermanentStorage;
}

TransactionState::~TransactionState() {
}

/*!
 * Returns TransactionState with awakening timestamp set to current UTC;
 */
TransactionState::SharedConst TransactionState::awakeAsFastAsPossible() {

    return TransactionState::SharedConst(
        new TransactionState(
            microsecondsTimestamp(timestamp())
        )
    );
}

/*!
 * Returns TransactionState with awakening timestamp set to current UTC + timeout;
 */
TransactionState::SharedConst TransactionState::awakeAfterTimeout(
    MicrosecondsTimestamp microseconds) {

    return TransactionState::SharedConst(
        new TransactionState(
            microsecondsTimestamp(timestamp() + posix::microseconds(microseconds))
        )
    );
}

const MicrosecondsTimestamp TransactionState::awakeningTimestamp() const {

    return mAwakeningTimestamp;
}

const vector<Message::MessageTypeID>& TransactionState::acceptedMessagesTypes() const {

    return mRequiredMessageTypes;
}

const bool TransactionState::needSerialize() const {

    return mFlushToPermanentStorage;
}




