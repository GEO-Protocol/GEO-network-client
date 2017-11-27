#include "ConfirmationRequiredMessagesHandler.h"


ConfirmationRequiredMessagesHandler::ConfirmationRequiredMessagesHandler(
    IOService &ioService,
    CommunicatorStorageHandler *communicatorStorageHandler,
    Logger &logger)
    noexcept:

    LoggerMixin(logger),
    mCommunicatorStorageHandler(communicatorStorageHandler),
    mIOService(ioService),
    mCleaningTimer(ioService)
{
    mDeserializationMessagesTimer = make_unique<as::steady_timer>(
        mIOService);
    mDeserializationMessagesTimer->expires_from_now(
        chrono::seconds(
            15));
    mDeserializationMessagesTimer->async_wait(
        boost::bind(
            &ConfirmationRequiredMessagesHandler::deserializeMessages,
            this));
}

void ConfirmationRequiredMessagesHandler::tryEnqueueMessage(
    const NodeUUID &contractorUUID,
    const Message::Shared message)
{
    if (message->typeID() == Message::TrustLines_SetIncoming
        /* and <other message type here> */) {

        // Appropriate message occurred and must be enqueued.
        // In case if no queue is present for this contractor - new one must be created.
        if (mQueues.count(contractorUUID) == 0) {
            auto newQueue = make_shared<ConfirmationRequiredMessagesQueue>(
                contractorUUID);
            newQueue->signalSaveMessageToStorage.connect(
                boost::bind(
                    &ConfirmationRequiredMessagesHandler::addMessageToStorage,
                    this,
                    _1,
                    _2));
            newQueue->signalRemoveMessageFromStorage.connect(
                boost::bind(
                    &ConfirmationRequiredMessagesHandler::removeMessageFromStorage,
                    this,
                    _1,
                    _2));
            mQueues[contractorUUID] = newQueue;
        }

        ioTransactionUnique = mCommunicatorStorageHandler->beginTransactionUnique();
        mQueues[contractorUUID]->enqueue(
            static_pointer_cast<TransactionMessage>(message));
        ioTransactionUnique = nullptr;

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

        auto ioTransaction = mCommunicatorStorageHandler->beginTransaction();
        ioTransaction->communicatorMessagesQueueHandler()->deleteRecord(
            contractorUUID,
            confirmationMessage->transactionUUID());

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        debug() << "Confirmation for message with transaction UUID " << confirmationMessage->transactionUUID() << " received. "
                << "Relevant message successfully confirmed.";
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

void ConfirmationRequiredMessagesHandler::addMessageToStorage(
    const NodeUUID &contractorUUID,
    Message::Shared message)
{
    auto bufferAndSize = message->serializeToBytes();
    ioTransactionUnique->communicatorMessagesQueueHandler()->saveRecord(
        contractorUUID,
        (static_pointer_cast<TransactionMessage>(message))->transactionUUID(),
        message->typeID(),
        bufferAndSize.first,
        bufferAndSize.second);
}

void ConfirmationRequiredMessagesHandler::removeMessageFromStorage(
    const NodeUUID &contractorUUID,
    Message::SerializedType messageType)
{
    ioTransactionUnique->communicatorMessagesQueueHandler()->deleteRecord(
        contractorUUID,
        messageType);
}

void ConfirmationRequiredMessagesHandler::deserializeMessages()
{
    mDeserializationMessagesTimer->cancel();
    vector<tuple<const NodeUUID, BytesShared, uint16_t>> messages;
    {
        auto ioTransaction = mCommunicatorStorageHandler->beginTransaction();
        messages = ioTransaction->communicatorMessagesQueueHandler()->allMessages();
    }
    this->info() << "Serialized messages count: " << to_string(messages.size());
    for (auto message : messages) {
        NodeUUID contractorUUID = NodeUUID::empty();
        BytesShared messageBody;
        uint16_t messageType;
        TransactionMessage::Shared sendingMessage;
        std::tie(contractorUUID, messageBody, messageType) = message;
        switch (messageType) {
            case Message::TrustLines_SetIncoming:
                sendingMessage = static_pointer_cast<TransactionMessage>(
                    make_shared<SetIncomingTrustLineMessage>(messageBody));
                break;
            default:
                mLog.error("ConfirmationRequiredMessagesHandler::deserializeMessages "
                               "invalid message type");
                continue;
        }
        tryEnqueueMessageWithoutConnectingSignalsToSlots(
            contractorUUID,
            sendingMessage);
    }
    for (auto contractorAndQueue : mQueues) {
        contractorAndQueue.second->signalSaveMessageToStorage.connect(
            boost::bind(
                &ConfirmationRequiredMessagesHandler::addMessageToStorage,
                this,
                _1,
                _2));
        contractorAndQueue.second->signalRemoveMessageFromStorage.connect(
            boost::bind(
                &ConfirmationRequiredMessagesHandler::removeMessageFromStorage,
                this,
                _1,
                _2));
    }
}

void ConfirmationRequiredMessagesHandler::tryEnqueueMessageWithoutConnectingSignalsToSlots(
        const NodeUUID &contractorUUID,
        const Message::Shared message)
{
    if (message->typeID() == Message::TrustLines_SetIncoming
        /* and <other message type here> */) {

        // Appropriate message occurred and must be enqueued.
        // In case if no queue is present for this contractor - new one must be created.
        if (mQueues.count(contractorUUID) == 0) {
            auto newQueue = make_shared<ConfirmationRequiredMessagesQueue>(
                contractorUUID);
            mQueues[contractorUUID] = newQueue;
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
