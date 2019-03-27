#include "ConfirmationRequiredMessagesQueue.h"


ConfirmationRequiredMessagesQueue::ConfirmationRequiredMessagesQueue(
    const SerializedEquivalent equivalent,
    ContractorID contractorID)
    noexcept:
    mEquivalent(equivalent),
    mContractorID(contractorID)
{
    resetInternalTimeout();
    mNextSendingAttemptDateTime = utc_now() + boost::posix_time::seconds(mNextTimeoutSeconds);
}

bool ConfirmationRequiredMessagesQueue::enqueue(
    TransactionMessage::Shared message)
{
    switch (message->typeID()) {
        case Message::Channel_Init: {
            addChannelInitInTheQueue(
                message);
            break;
        }
        case Message::GatewayNotification: {
            updateGatewayNotificationInTheQueue(
                message);
            break;
        }
        default:
            return false;
    }
    resetInternalTimeout();
    mNextSendingAttemptDateTime = utc_now() + boost::posix_time::seconds(mNextTimeoutSeconds);
    return true;
}

bool ConfirmationRequiredMessagesQueue::tryProcessConfirmation(
    ConfirmationMessage::Shared confirmationMessage)
{
    if (mMessages.erase(confirmationMessage->transactionUUID()) > 0) {
        // Remote node responded with the valid response.
        // Internal timeout should be reset.
        resetInternalTimeout();
        return true;
    }

    return false;
}

const DateTime &ConfirmationRequiredMessagesQueue::nextSendingAttemptDateTime()
    noexcept
{
    return mNextSendingAttemptDateTime;
}

const map<TransactionUUID, TransactionMessage::Shared> &ConfirmationRequiredMessagesQueue::messages()
    noexcept
{
    // Exponentially increase re-sending timeout up to 10+ minutes.
    if (mNextTimeoutSeconds < 60 * 10) {
        mNextTimeoutSeconds *= 2;
    }

    mNextSendingAttemptDateTime = utc_now() + boost::posix_time::seconds(mNextTimeoutSeconds);

    return mMessages;
}

const size_t ConfirmationRequiredMessagesQueue::size() const
    noexcept
{
    return mMessages.size();
}

void ConfirmationRequiredMessagesQueue::resetInternalTimeout()
    noexcept
{
    mNextTimeoutSeconds = 4;
}

void ConfirmationRequiredMessagesQueue::addChannelInitInTheQueue(
    TransactionMessage::Shared message)
{
    mMessages[message->transactionUUID()] = message;
    signalSaveMessageToStorage(
        mContractorID,
        message);

}

void ConfirmationRequiredMessagesQueue::updateGatewayNotificationInTheQueue(
    TransactionMessage::Shared message)
{
    // Only one GatewayNotificationMessage should be in the queue in one moment of time.
    // queue must contains only newest one notification, all other must be removed.
    for (auto it = mMessages.cbegin(); it != mMessages.cend();) {
        const auto kMessage = it->second;

        if (kMessage->typeID() == Message::GatewayNotification) {
            mMessages.erase(it++);
        } else {
            ++it;
        }
    }

    mMessages[message->transactionUUID()] = message;
}
