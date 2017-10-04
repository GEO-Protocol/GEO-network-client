#include "ConfirmationRequiredMessagesHandler.h"


ConfirmationRequiredMessagesHandler::ConfirmationRequiredMessagesHandler(
    IOService &ioService,
    Logger &logger)
    noexcept:

    LoggerMixin(logger),
    mIOService(ioService),
    mCleaningTimer(ioService)
{}

void ConfirmationRequiredMessagesHandler::tryEnqueueMessage(
    const NodeUUID &contractorUUID,
    const Message::Shared message)
{
    if (message->typeID() == Message::TrustLines_SetIncoming
        /* and <other message type here> */) {

        // Appropriate message occured and must be enqueued.
        // In case if no queue is present for this contractor - new one must be created.
        if (mQueues.count(contractorUUID) == 0) {
            mQueues[contractorUUID] = make_shared<ConfirmationRequiredMessagesQueue>();
        }

        mQueues[contractorUUID]->enqueue(
            static_pointer_cast<TransactionMessage>(message));

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        debug() << "Message of type " << message->typeID() << " enqueued for confirmation receiving.";
#endif

        if (mQueues.size() == 1
            and mQueues.begin()->second->size() == 1) {

            // First message was added for further re-sending.
            rescheduleResending();
        }
    }
}

void ConfirmationRequiredMessagesHandler::tryProcessConfirmation(
    const NodeUUID &contractorUUID,
    const ConfirmationMessage::Shared confirmationMessage)
{
    if (mQueues.count(contractorUUID) == 0) {
        // No queue is present for this contractor.
        // No enqueued messages are present.
        return;
    }

    auto queue = mQueues[contractorUUID];
    if (queue->tryProcessConfirmation(confirmationMessage)) {

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        debug() << "Confirmation for message with transaction UUID " << confirmationMessage->transactionUUID() << " received. "
                << "Relevant message succesfully confirmed.";
#endif

        // In case if last message was removed from the queue -
        // the queue itself must be removed too.
        if (queue->size() == 0) {
            mQueues.erase(contractorUUID);
        }
    }
}

const string ConfirmationRequiredMessagesHandler::logHeader() const
    noexcept
{
    return "[ConfirmationRequiredMessagesHandler]";
}

const DateTime ConfirmationRequiredMessagesHandler::closestQueueSendingTimestamp() const
    noexcept
{
    DateTime nextClearingDateTime = utc_now() + boost::posix_time::seconds(2);
    bool nextClearingDateTimeInitialisedWithFirstTimestamp = false;

    for (const auto &contractorUUIDAndQueue : mQueues) {
        const auto kQueueNextAttemptPlanned = contractorUUIDAndQueue.second->nextSendingAttemptDateTime();

        if (not nextClearingDateTimeInitialisedWithFirstTimestamp) {
            nextClearingDateTime = kQueueNextAttemptPlanned;

        } else if (kQueueNextAttemptPlanned < nextClearingDateTime) {
            nextClearingDateTime = kQueueNextAttemptPlanned;
        }
    }

    return nextClearingDateTime;
}

void ConfirmationRequiredMessagesHandler::rescheduleResending()
{
    if (mQueues.size() == 0) {

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        this->debug() << "There are no postponed messages present. "
                         "Cleaning would not be scheduled any more.";
#endif

        return;
    }

    const auto kCleaningTimeout = closestQueueSendingTimestamp() - utc_now();
    mCleaningTimer.expires_from_now(kCleaningTimeout);
    mCleaningTimer.async_wait([this] (const boost::system::error_code &_) {

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

void ConfirmationRequiredMessagesHandler::sendPostponedMessages() const
{
    const auto now = utc_now();

    for (const auto &contractorUUIDAndQueue : mQueues) {
        const auto kContractor = contractorUUIDAndQueue.first;
        const auto kQueue = contractorUUIDAndQueue.second;

        if (kQueue->nextSendingAttemptDateTime() > now) {
            // This queue's timeout is not fired up yet.
            continue;
        }

        for (const auto transactionUUIDAndMessage : kQueue->messages()) {
            signalOutgoingMessageReady(
                make_pair(
                    kContractor,
                    transactionUUIDAndMessage.second));
        }
    }
}