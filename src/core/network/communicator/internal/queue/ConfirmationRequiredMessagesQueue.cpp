#include "ConfirmationRequiredMessagesQueue.h"


ConfirmationRequiredMessagesQueue::ConfirmationRequiredMessagesQueue(
    const NodeUUID &contractorUUID)
    noexcept:
    mContractorUUID(contractorUUID)
{
    resetInternalTimeout();
    mNextSendingAttemptDateTime = utc_now() + boost::posix_time::seconds(mNextTimeoutSeconds);
}

void ConfirmationRequiredMessagesQueue::enqueue(
    TransactionMessage::Shared message)
{
    switch (message->typeID()) {
        case Message::TrustLines_SetIncoming: {
            updateTrustLineNotificationInTheQueue(
                static_pointer_cast<SetIncomingTrustLineMessage>(message));
            break;
        }
        case Message::TrustLines_CloseOutgoing: {
            updateTrustLineCloseNotificationInTheQueue(
                static_pointer_cast<CloseOutgoingTrustLineMessage>(message));
            break;
        }
        case Message::GatewayNotification: {
            updateGatewayNotificationInTheQueue(
                static_pointer_cast<GatewayNotificationMessage>(message));
            break;
        }
        case Message::TrustLines_SetIncomingFromGateway: {
            updateTrustLineFromGatewayNotificationInTheQueue(
                static_pointer_cast<SetIncomingTrustLineFromGatewayMessage>(message));
            break;
        }
        case Message::GatewayNotificationOneEquivalent: {
            updateGatewayNotificationOneEquivalentInTheQueue(
                static_pointer_cast<GatewayNotificationOneEquivalentMessage>(message));
            break;
        }
        //todo : add logger and warning default case
    }
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
    mNextTimeoutSeconds = 2;
}

void ConfirmationRequiredMessagesQueue::updateTrustLineNotificationInTheQueue(
    SetIncomingTrustLineMessage::Shared message)
{
    // Only one SetIncomingTrustLineMessage should be in the queue in one moment of time.
    // queue must contains only newest one notification, all other must be removed.
    for (auto it = mMessages.cbegin(); it != mMessages.cend();) {
        const auto kMessage = it->second;

        if (kMessage->typeID() == Message::TrustLines_SetIncoming
            and kMessage->equivalent() == message->equivalent()) {
            mMessages.erase(it++);
            signalRemoveMessageFromStorage(
                mContractorUUID,
                kMessage->typeID());
        } else {
            ++it;
        }
    }

    mMessages[message->transactionUUID()] = message;
    signalSaveMessageToStorage(
        mContractorUUID,
        message);
}

void ConfirmationRequiredMessagesQueue::updateTrustLineFromGatewayNotificationInTheQueue(
    SetIncomingTrustLineFromGatewayMessage::Shared message)
{
    // Only one SetIncomingTrustLineFromGatewayMessage should be in the queue in one moment of time.
    // queue must contains only newest one notification, all other must be removed.
    for (auto it = mMessages.cbegin(); it != mMessages.cend();) {
        const auto kMessage = it->second;

        if (kMessage->typeID() == Message::TrustLines_SetIncomingFromGateway
            and kMessage->equivalent() == message->equivalent()) {
            mMessages.erase(it++);
            signalRemoveMessageFromStorage(
                mContractorUUID,
                kMessage->typeID());
        } else {
            ++it;
        }
    }

    mMessages[message->transactionUUID()] = message;
    signalSaveMessageToStorage(
        mContractorUUID,
        message);
}

// todo : discuss if need keep only one message in queue
void ConfirmationRequiredMessagesQueue::updateTrustLineCloseNotificationInTheQueue(
    CloseOutgoingTrustLineMessage::Shared message)
{
    // Only one CloseOutgoingTrustLineMessage should be in the queue in one moment of time.
    // queue must contains only newest one notification, all other must be removed.
    for (auto it = mMessages.cbegin(); it != mMessages.cend();) {
        const auto kMessage = it->second;

        if (kMessage->typeID() == Message::TrustLines_CloseOutgoing
            and kMessage->equivalent() == message->equivalent()) {
            mMessages.erase(it++);
            signalRemoveMessageFromStorage(
                mContractorUUID,
                kMessage->typeID());
        } else {
            ++it;
        }
    }

    mMessages[message->transactionUUID()] = message;
    signalSaveMessageToStorage(
        mContractorUUID,
        message);
}

void ConfirmationRequiredMessagesQueue::updateGatewayNotificationInTheQueue(
    GatewayNotificationMessage::Shared message)
{
    // Only one GatewayNotificationMessage should be in the queue in one moment of time.
    // queue must contains only newest one notification, all other must be removed.
    for (auto it = mMessages.cbegin(); it != mMessages.cend();) {
        const auto kMessage = it->second;

        if (kMessage->typeID() == Message::GatewayNotification) {
            mMessages.erase(it++);
            signalRemoveMessageFromStorage(
                mContractorUUID,
                kMessage->typeID());
        } else {
            ++it;
        }
    }

    mMessages[message->transactionUUID()] = message;
    signalSaveMessageToStorage(
        mContractorUUID,
        message);
}

void ConfirmationRequiredMessagesQueue::updateGatewayNotificationOneEquivalentInTheQueue(
    GatewayNotificationOneEquivalentMessage::Shared message)
{
    // Only one GatewayNotificationOneEquivalentMessage should be in the queue in one moment of time.
    // queue must contains only newest one notification, all other must be removed.
    for (auto it = mMessages.cbegin(); it != mMessages.cend();) {
        const auto kMessage = it->second;

        if (kMessage->typeID() == Message::GatewayNotificationOneEquivalent
            and kMessage->equivalent() == message->equivalent()) {
            mMessages.erase(it++);
            signalRemoveMessageFromStorage(
                mContractorUUID,
                kMessage->typeID());
        } else {
            ++it;
        }
    }

    mMessages[message->transactionUUID()] = message;
    signalSaveMessageToStorage(
        mContractorUUID,
        message);
}
