#include "TransactionState.h"

/*!
 * Returns TransactionState that simply closes the transaction.
 */
TransactionState::Shared TransactionState::exit() {
    // WARN:
    // Do not use 0 as value for awakeningTimestamp!
    // It will break scheduler logic for choosing next transaction for execution.
    return make_shared<TransactionState>(
        numeric_limits<GEOEpochTimestamp>::max());
}

/*!
 * Returns TransactionState with awakening timestamp set to current UTC;
 */
TransactionState::Shared TransactionState::awakeAsFastAsPossible() {
    auto t = utc_now();
    return make_shared<TransactionState>(
        microsecondsSinceGEOEpoch(
            t));
}

/*!
 * Returns TransactionState with awakening timestamp set to current UTC + timeout;
 */
TransactionState::Shared TransactionState::awakeAfterMilliseconds(
    uint16_t milliseconds) {

    auto t = utc_now() + pt::microseconds(milliseconds*1000);
    return make_shared<TransactionState>(
        microsecondsSinceGEOEpoch(
            t));
}

/*!
 * Returns TransactionState that specifies what kind of messages transaction is waiting and accepting.
 * Optionally, may be initialised with deadline timeout.
 */
TransactionState::Shared TransactionState::waitForMessageTypes(
    vector<Message::MessageTypeID> &&requiredMessageType,
    uint16_t noLongerThanMilliseconds) {

    TransactionState::Shared state;
    if (noLongerThanMilliseconds == 0) {
        state = TransactionState::exit();

    } else {
        state = TransactionState::awakeAfterMilliseconds(
            noLongerThanMilliseconds);
    }

    state->mRequiredMessageTypes = requiredMessageType;
    return state;
}

TransactionState::TransactionState(
    GEOEpochTimestamp awakeningTimestamp,
    bool flushToPermanentStorage) :

    mAwakeningTimestamp(awakeningTimestamp) {

    mFlushToPermanentStorage = flushToPermanentStorage;
}

TransactionState::TransactionState(
    Message::MessageTypeID requiredMessageType,
    bool flushToPermanentStorage) {

    mRequiredMessageTypes.push_back(requiredMessageType);
    mFlushToPermanentStorage = flushToPermanentStorage;
}

TransactionState::TransactionState(
    GEOEpochTimestamp awakeningTimestamp,
    Message::MessageTypeID requiredMessageType,
    bool flushToPermanentStorage) :

        mAwakeningTimestamp(awakeningTimestamp) {

    mRequiredMessageTypes.push_back(requiredMessageType);
    mFlushToPermanentStorage = flushToPermanentStorage;
}

const GEOEpochTimestamp TransactionState::awakeningTimestamp() const {
    return mAwakeningTimestamp;
}

const vector<Message::MessageTypeID>& TransactionState::acceptedMessagesTypes() const {
    return mRequiredMessageTypes;
}

const bool TransactionState::needSerialize() const {
    return mFlushToPermanentStorage;
}

const bool TransactionState::mustBeRescheduled() const {
    return
        (mAwakeningTimestamp != numeric_limits<GEOEpochTimestamp>::max()) ||
        (acceptedMessagesTypes().size() > 0);
}

const bool TransactionState::mustExit() const {
    return !mustBeRescheduled();
}
