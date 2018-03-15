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
    if (message->typeID() == Message::MaxFlow_ResultMaxFlowCalculation
        or message->typeID() == Message::MaxFlow_ResultMaxFlowCalculationFromGateway
        /* or <other message type here> */) {

        // Appropriate message occurred and must be enqueued.
        // In case if no queue is present for this contractor - new one must be created.
        if (mQueues.count(contractorUUID) == 0) {
            auto newQueue = make_shared<ConfirmationNotStronglyRequiredMessagesQueue>(
                contractorUUID);
            mQueues[contractorUUID] = newQueue;
        }

        mQueues[contractorUUID]->enqueue(
            static_pointer_cast<MaxFlowCalculationConfirmationMessage>(message),
            mCurrentConfirmationID);
        mCurrentConfirmationID++;

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        debug() << "Message of type " << message->typeID() << " enqueued for not strongly confirmation receiving.";
#endif

        if (mQueues.size() == 1
            and mQueues.begin()->second->size() == 1) {

            // First message was added for further re-sending.
            rescheduleResending();
        }
    }
}

void ConfirmationNotStronglyRequiredMessagesHandler::tryProcessConfirmation(
    const NodeUUID &contractorUUID,
    const MaxFlowCalculationConfirmationMessage::Shared confirmationMessage)
{
    if (mQueues.count(contractorUUID) == 0) {
        warning() << "tryProcessConfirmation: no queue is present for contractor " << contractorUUID;
        return;
    }

    auto queue = mQueues[contractorUUID];
    if (queue->tryProcessConfirmation(confirmationMessage)) {

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        debug() << "Confirmation for message with ConfirmationID " << confirmationMessage->confirmationID() << " received. "
                << "Relevant message successfully confirmed.";
#endif

        // In case if last message was removed from the queue -
        // the queue itself must be removed too.
        if (queue->size() == 0) {
            mQueues.erase(contractorUUID);
        }
    } else {
        warning() << "tryProcessConfirmation: can't process";
    }
}

const string ConfirmationNotStronglyRequiredMessagesHandler::logHeader() const
    noexcept
{
    return "[ConfirmationNotStronglyRequiredMessagesHandler]";
}

const DateTime ConfirmationNotStronglyRequiredMessagesHandler::closestQueueSendingTimestamp() const
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
        auto kContractor = contractorUUIDAndQueue.first;
        const auto kQueue = contractorUUIDAndQueue.second;

        if (!kQueue->checkIfNeedResendMessages()) {
            signalClearTopologyCache(kContractor);
            mQueues.erase(kContractor);
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
