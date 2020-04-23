#include "ConfirmationNotStronglyRequiredMessagesQueue.h"

ConfirmationNotStronglyRequiredMessagesQueue::ConfirmationNotStronglyRequiredMessagesQueue(
    const SerializedEquivalent equivalent,
    BaseAddress::Shared contractorAddress,
    Logger &logger)
    noexcept:
    LoggerMixin(logger),
    mEquivalent(equivalent),
    mContractorAddress(contractorAddress)
{
    resetInternalTimeout();
    mNextSendingAttemptDateTime = utc_now() + boost::posix_time::seconds(mNextTimeoutSeconds);
}

bool ConfirmationNotStronglyRequiredMessagesQueue::enqueue(
    MaxFlowCalculationConfirmationMessage::Shared message,
    ConfirmationID confirmationID)
{
    // todo : check all sender addresses
    if (message->equivalent() != mEquivalent) {
        this->warning() << "Message doesn't belong to this queue due to equivalent: " << message->equivalent();
        return false;
    }
    message->setConfirmationID(
        confirmationID);
    switch (message->typeID()) {
        case Message::MaxFlow_ResultMaxFlowCalculation:
        case Message::MaxFlow_ResultMaxFlowCalculationFromGateway: {
            if (mMessages.count(confirmationID) != 0) {
                this->warning() << "Message with confirmationID " << confirmationID
                                << " has already present in queue";
                return false;
            }
            mMessages[message->confirmationID()] = message;
            return true;
        }
        default:
            this->warning() << "Invalid  message type " << message->typeID();
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

BaseAddress::Shared ConfirmationNotStronglyRequiredMessagesQueue::contractorAddress() const
{
    return mContractorAddress;
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
    mNextTimeoutSeconds = 4;
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

const string ConfirmationNotStronglyRequiredMessagesQueue::logHeader() const
noexcept
{
    stringstream ss;
    ss << "[ConfirmationNotStronglyRequiredMessagesQueue " << mContractorAddress->fullAddress() << " ]";
    return ss.str();
}