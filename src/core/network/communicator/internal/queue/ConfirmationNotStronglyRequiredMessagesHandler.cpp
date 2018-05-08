#include "ConfirmationNotStronglyRequiredMessagesHandler.h"

ConfirmationNotStronglyRequiredMessagesHandler::ConfirmationNotStronglyRequiredMessagesHandler(
    IOService &ioService,
    Logger &logger)
    noexcept:

    LoggerMixin(logger),
    mIOService(ioService),
    mCleaningTimer(ioService),
    mCurrentConfirmationID(0)
{}

void ConfirmationNotStronglyRequiredMessagesHandler::tryEnqueueMessage(
    const NodeUUID &contractorUUID,
    const Message::Shared message)
{
    // Only messages on which method isAddToConfirmationNotStronglyRequiredMessagesHandler returns true
    // can be enqueued, so if you want add message to ConfirmationNotStronglyRequiredMessagesHandler,
    // you should override this method
    // In case if no queue is present for this contractor - new one must be created.
    const auto equivalent = message->equivalent();
    const auto queueKey = make_pair(
        equivalent,
        contractorUUID);
    if (mQueues.count(queueKey) == 0) {
        auto newQueue = make_shared<ConfirmationNotStronglyRequiredMessagesQueue>(
            equivalent,
            contractorUUID);
        mQueues[queueKey] = newQueue;
    }

    if (!mQueues[queueKey]->enqueue(
        static_pointer_cast<MaxFlowCalculationConfirmationMessage>(message),
        mCurrentConfirmationID)) {
        warning() << "tryEnqueueMessage: can't enqueue message "
                  << message->typeID() << " with confirmation id " << mCurrentConfirmationID;
    }
    mCurrentConfirmationID++;

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    debug() << "Message of type " << message->typeID() << " for equivalent " << equivalent
            << " enqueued for not strongly confirmation receiving.";
#endif

    if (mQueues.size() == 1
        and mQueues.begin()->second->size() == 1) {

        // First message was added for further re-sending.
        rescheduleResending();
    }
}

void ConfirmationNotStronglyRequiredMessagesHandler::tryProcessConfirmation(
    const MaxFlowCalculationConfirmationMessage::Shared confirmationMessage)
{
    const auto queueKey = make_pair(
        confirmationMessage->equivalent(),
        confirmationMessage->senderUUID);
    if (mQueues.count(queueKey) == 0) {
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        warning() << "tryProcessConfirmation: no queue is present for contractor "
                  << queueKey.second << " on equivalent " << queueKey.first;
#endif
        return;
    }

    auto queue = mQueues[queueKey];
    if (queue->tryProcessConfirmation(confirmationMessage)) {

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        debug() << "Confirmation for message with ConfirmationID " << confirmationMessage->confirmationID() << " received. "
                << "Relevant message successfully confirmed.";
#endif

        // In case if last message was removed from the queue -
        // the queue itself must be removed too.
        if (queue->size() == 0) {
            mQueues.erase(queueKey);
        }
    } else {
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        warning() << "tryProcessConfirmation: can't process";
#endif
    }
}

const DateTime ConfirmationNotStronglyRequiredMessagesHandler::closestQueueSendingTimestamp() const
    noexcept
{
    if (mQueues.empty()) {
        return utc_now() + boost::posix_time::seconds(2);
    }

    DateTime nextClearingDateTime = mQueues.begin()->second->nextSendingAttemptDateTime();
    for (const auto &contractorUUIDAndQueue : mQueues) {
        const auto kQueueNextAttemptPlanned = contractorUUIDAndQueue.second->nextSendingAttemptDateTime();
        if (kQueueNextAttemptPlanned < nextClearingDateTime) {
            nextClearingDateTime = kQueueNextAttemptPlanned;
        }
    }
    return nextClearingDateTime;
}

void ConfirmationNotStronglyRequiredMessagesHandler::rescheduleResending()
{
    if (mQueues.empty()) {

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        this->debug() << "There are no postponed messages present. "
                         "Cleaning would not be scheduled any more.";
#endif

        return;
    }

    const auto kCleaningTimeout = closestQueueSendingTimestamp() - utc_now();
    mCleaningTimer.expires_from_now(chrono::microseconds(kCleaningTimeout.total_microseconds()));
    mCleaningTimer.async_wait([this] (const boost::system::error_code &e) {

        if (e == boost::asio::error::operation_aborted) {
            return;
        }

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        this->debug() << "Enqueued messages re-sending started.";
#endif

        this->sendPostponedMessages();
        this->rescheduleResending();

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        this->debug() << "Enqueued messages re-sending finished.";
#endif
    });
}

void ConfirmationNotStronglyRequiredMessagesHandler::sendPostponedMessages()
{
    const auto now = utc_now();

    for (const auto &contractorUUIDAndQueue : mQueues) {
        auto kEquivalent = contractorUUIDAndQueue.first.first;
        auto kContractor = contractorUUIDAndQueue.first.second;
        const auto kQueue = contractorUUIDAndQueue.second;

        if (!kQueue->checkIfNeedResendMessages()) {
            signalClearTopologyCache(
                kEquivalent,
                kContractor);
            mQueues.erase(contractorUUIDAndQueue.first);
            continue;
        }

        if (kQueue->nextSendingAttemptDateTime() > now) {
            // This queue's timeout is not fired up yet.
            continue;
        }

        for (const auto &confirmationIDAndMessage : kQueue->messages()) {
            signalOutgoingMessageReady(
                make_pair(
                    kContractor,
                    confirmationIDAndMessage.second));
        }
    }
}

const string ConfirmationNotStronglyRequiredMessagesHandler::logHeader() const
    noexcept
{
    return "[ConfirmationNotStronglyRequiredMessagesHandler]";
}
