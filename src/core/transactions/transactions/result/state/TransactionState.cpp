#include "TransactionState.h"

TransactionState::TransactionState(
    Milliseconds timeout,
    bool flushToPermanentStorage) :

    mAwakeningTimestamp(microsecondsTimestamp(now() + posix::microseconds(timeout * 1000))) {

    mFlushToPermanentStorage = flushToPermanentStorage;
}

TransactionState::TransactionState(
    Message::MessageTypeID requiredMessageType,
    bool flushToPermanentStorage) {

    mRequiredMessageTypes.push_back(requiredMessageType);
    mFlushToPermanentStorage = flushToPermanentStorage;
}

TransactionState::TransactionState(
    Milliseconds timeout,
    Message::MessageTypeID requiredMessageType,
    bool flushToPermanentStorage) :

    mAwakeningTimestamp(microsecondsTimestamp(now() + posix::microseconds(timeout * 1000))) {

    mRequiredMessageTypes.push_back(requiredMessageType);
    mFlushToPermanentStorage = flushToPermanentStorage;
}

TransactionState::~TransactionState() {}

/*!
 * Returns TransactionState that simply closes the transaction.
 */
TransactionState::SharedConst TransactionState::exit() {

    return TransactionState::SharedConst(
        new TransactionState(0)
    );
}

/*!
 * Returns TransactionState with awakening timestamp set to current UTC;
 */
TransactionState::SharedConst TransactionState::awakeAsFastAsPossible() {

    return TransactionState::SharedConst(
        new TransactionState(0)
    );
}

/*!
 * Returns TransactionState with awakening timestamp set to current UTC + timeout;
 */
TransactionState::SharedConst TransactionState::awakeAfterMilliseconds(
    Milliseconds milliseconds) {

    return TransactionState::SharedConst(
        new TransactionState(
            milliseconds
        )
    );
}

/*!
 * Returns TransactionState that specifies what kind of mesages transaction is waiting and accepting.
 * Optionally, may be initialised with deadline timeout.
 */
TransactionState::SharedConst TransactionState::waitForMessageTypes(
    vector<Message::MessageTypeID> &&requiredMessageType,
    Milliseconds noLongerThanMilliseconds) {

    TransactionState::Shared state;
    if (noLongerThanMilliseconds == 0) {
        state = const_pointer_cast<TransactionState> (TransactionState::exit());

    } else {
        state = const_pointer_cast<TransactionState> (TransactionState::awakeAfterMilliseconds(
            noLongerThanMilliseconds
        ));
    }

    state->mRequiredMessageTypes = requiredMessageType;

    return const_pointer_cast<const TransactionState>(state);
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

const bool TransactionState::mustBeRescheduled() const {

    return (mAwakeningTimestamp != 0) || (acceptedMessagesTypes().size() > 0);
}




