#include "TransactionState.h"

TransactionState::TransactionState(
    Message::MessageType requiredMessageType,
    bool flushToPermanentStorage,
    bool awakeOnMessage) :

    mAwakeningTimestamp(0),
    mFlushToPermanentStorage(flushToPermanentStorage),
    mMustBeAwakenedOnMessage(awakeOnMessage),
    mMustSavePreviousStateState(false)
{
    mRequiredMessageTypes.push_back(requiredMessageType);
}

TransactionState::TransactionState(
    GEOEpochTimestamp awakeningTimestamp,
    bool flushToPermanentStorage,
    bool awakeOnMessage) :

    mFlushToPermanentStorage(flushToPermanentStorage),
    mMustBeAwakenedOnMessage(awakeOnMessage),
    mAwakeningTimestamp(awakeningTimestamp),
    mMustSavePreviousStateState(false)
{}

TransactionState::TransactionState(
    GEOEpochTimestamp awakeningTimestamp,
    Message::MessageType requiredMessageType,
    bool flushToPermanentStorage,
    bool awakeOnMessage) :

    mFlushToPermanentStorage(flushToPermanentStorage),
    mMustBeAwakenedOnMessage(awakeOnMessage),
    mAwakeningTimestamp(awakeningTimestamp),
    mMustSavePreviousStateState(false)
{
    mRequiredMessageTypes.push_back(requiredMessageType);
}

TransactionState::TransactionState(
    bool mustSavePreviousState) :

    mMustSavePreviousStateState(mustSavePreviousState)
{}

/*!
 * Returns TransactionState that simply closes the transaction.
 *
 * WARNING:
 * Do not use 0 as value for awakeningTimestamp.
 * It will break scheduler logic for choosing next transaction for execution.
 */
TransactionState::SharedConst TransactionState::exit()
{
    return make_shared<TransactionState>(
        numeric_limits<GEOEpochTimestamp>::max());
}

TransactionState::SharedConst TransactionState::flushAndContinue()
{
    return make_shared<TransactionState>(
        microsecondsSinceGEOEpoch(
            utc_now()),
        true);
}

/*!
 * Returns TransactionState with awakening timestamp set to current UTC;
 */
TransactionState::SharedConst TransactionState::awakeAsFastAsPossible()
{
    return make_shared<TransactionState>(
        microsecondsSinceGEOEpoch(
            utc_now()));
}

/*!
 * Returns TransactionState with awakening timestamp set to current UTC + timeout;
 */
TransactionState::SharedConst TransactionState::awakeAfterMilliseconds(
    uint32_t milliseconds)
{
    return make_shared<TransactionState>(
        microsecondsSinceGEOEpoch(
            utc_now() + pt::microseconds(milliseconds * 1000)));
}

/*!
 * Returns TransactionState that specifies what kind of messages transaction is waiting and accepting.
 * Optionally, may be initialised with deadline timeout.
 */
TransactionState::SharedConst TransactionState::waitForMessageTypes(
    vector<Message::MessageType> &&requiredMessageType,
    uint32_t noLongerThanMilliseconds)
{
    TransactionState::Shared state;
    if (noLongerThanMilliseconds == 0) {
        state = const_pointer_cast<TransactionState> (TransactionState::exit());

    } else {
        state = const_pointer_cast<TransactionState> (TransactionState::awakeAfterMilliseconds(
            noLongerThanMilliseconds));
    }

    state->mRequiredMessageTypes = requiredMessageType;

    return const_pointer_cast<const TransactionState>(state);
}

TransactionState::SharedConst TransactionState::waitForMessageTypesAndAwakeAfterMilliseconds(
    vector<Message::MessageType> &&requiredMessageType,
    uint32_t noLongerThanMilliseconds)
{
    TransactionState::Shared state;
    if (noLongerThanMilliseconds == 0) {
        state = const_pointer_cast<TransactionState> (TransactionState::exit());

    } else {
        state = make_shared<TransactionState>(
            microsecondsSinceGEOEpoch(
                utc_now() + pt::microseconds(noLongerThanMilliseconds * 1000)),
            false,
            false);
    }

    state->mRequiredMessageTypes = requiredMessageType;

    return const_pointer_cast<const TransactionState>(state);
}

TransactionState::SharedConst TransactionState::waitForResourcesTypes(
    vector<BaseResource::ResourceType> &&requiredResourcesType,
    uint32_t noLongerThanMilliseconds)
{
    TransactionState::Shared state;
    if (noLongerThanMilliseconds == 0) {
        state = const_pointer_cast<TransactionState> (TransactionState::exit());

    } else {
        state = const_pointer_cast<TransactionState> (TransactionState::awakeAfterMilliseconds(
            noLongerThanMilliseconds));
    }

    state->mRequiredResourcesTypes = requiredResourcesType;

    return const_pointer_cast<const TransactionState>(state);
}

TransactionState::SharedConst TransactionState::continueWithPreviousState()
{
    TransactionState::Shared state;
    state = make_shared<TransactionState>(true);
    return const_pointer_cast<const TransactionState>(state);
}

const GEOEpochTimestamp TransactionState::awakeningTimestamp() const
{
    return mAwakeningTimestamp;
}

const vector<Message::MessageType>& TransactionState::acceptedMessagesTypes() const
{
    return mRequiredMessageTypes;
}

const vector<BaseResource::ResourceType> &TransactionState::acceptedResourcesTypes() const
{
    return mRequiredResourcesTypes;
}

const bool TransactionState::needSerialize() const
{
    return mFlushToPermanentStorage;
}

const bool TransactionState::mustBeRescheduled() const
{
    return
        (mAwakeningTimestamp != numeric_limits<GEOEpochTimestamp>::max()) ||
        (!acceptedMessagesTypes().empty());
}

const bool TransactionState::mustExit() const
{
    return !mustBeRescheduled();
}

const bool TransactionState::mustBeAwakenedOnMessage() const
{
    return mMustBeAwakenedOnMessage;
}

const bool TransactionState::mustSavePreviousStateState() const
{
    return mMustSavePreviousStateState;
}
