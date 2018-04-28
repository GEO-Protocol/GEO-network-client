#include "ConfirmationNotStronglyRequiredMessagesQueue.h"

ConfirmationNotStronglyRequiredMessagesQueue::ConfirmationNotStronglyRequiredMessagesQueue(
    const SerializedEquivalent equivalent,
    const NodeUUID &contractorUUID)
    noexcept:
    mEquivalent(equivalent),
    mContractorUUID(contractorUUID)
{
    resetInternalTimeout();
    mNextSendingAttemptDateTime = utc_now() + boost::posix_time::seconds(mNextTimeoutSeconds);
}

bool ConfirmationNotStronglyRequiredMessagesQueue::enqueue(
    MaxFlowCalculationConfirmationMessage::Shared message,
    ConfirmationID confirmationID)
{
    message->setConfirmationID(
        confirmationID);
    switch (message->typeID()) {
        case Message::MaxFlow_ResultMaxFlowCalculation:
        case Message::MaxFlow_ResultMaxFlowCalculationFromGateway: {
            if (mMessages.count(confirmationID) != 0) {
                return false;
            }
            mMessages[message->confirmationID()] = message;
            return true;
        }
        default:
            return false;
    }
}

bool ConfirmationNotStronglyRequiredMessagesQueue::tryProcessConfirmation(
    MaxFlowCalculationConfirmationMessage::Shared confirmationMessage)
{
    if (mMessages.erase(confirmationMessage->confirmationID()) > 0) {
        // Remote node responded with the valid response.
        // Internal timeout should be reset.
        resetInternalTimeout();
        return true;
    }

    return false;
}

const DateTime &ConfirmationNotStronglyRequiredMessagesQueue::nextSendingAttemptDateTime()
    noexcept
{
    return mNextSendingAttemptDateTime;
}

const map<ConfirmationID, MaxFlowCalculationConfirmationMessage::Shared> &ConfirmationNotStronglyRequiredMessagesQueue::messages()
    noexcept
{
    mNextSendingAttemptDateTime = utc_now() + boost::posix_time::seconds(mNextTimeoutSeconds);
    return mMessages;
}

const size_t ConfirmationNotStronglyRequiredMessagesQueue::size() const
    noexcept
{
    return mMessages.size();
}

void ConfirmationNotStronglyRequiredMessagesQueue::resetInternalTimeout()
    noexcept
{
    mNextTimeoutSeconds = 3;
    mCountResendingAttempts = 0;
}

bool ConfirmationNotStronglyRequiredMessagesQueue::checkIfNeedResendMessages()
{
    mCountResendingAttempts++;
    if (mCountResendingAttempts > kMaxCountResendingAttempts) {
        mMessages.clear();
        return false;
    }
    return true;
}