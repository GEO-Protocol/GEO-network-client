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
    deserializeMessages();
}

void ConfirmationRequiredMessagesHandler::tryEnqueueMessage(
    const NodeUUID &contractorUUID,
    const Message::Shared message)
{
    // Only messages on which method isAddToConfirmationRequiredMessagesHandler returns true
    // can be enqueued, so if you want add message to ConfirmationRequiredMessagesHandler,
    // you should override this method
    // In case if no queue is present for this contractor - new one must be created.
    const auto queueKey = make_pair(
        message->equivalent(),
        contractorUUID);
    if (mQueues.count(queueKey) == 0) {
        auto newQueue = make_shared<ConfirmationRequiredMessagesQueue>(
            queueKey.first,
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
                _2,
                _3));
        mQueues[queueKey] = newQueue;
    }

    ioTransactionUnique = mCommunicatorStorageHandler->beginTransactionUnique();
    if (!mQueues[queueKey]->enqueue(
        static_pointer_cast<TransactionMessage>(message))) {
        warning() << "Can't enqueue message, invalid message type " << message->typeID();
        if (mQueues[queueKey]->size() == 0) {
            mQueues.erase(queueKey);
        }
    }
    ioTransactionUnique = nullptr;

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    debug() << "Message of type " << message->typeID() << " for equivalent " << message->equivalent()
            << " enqueued for confirmation receiving.";
#endif

    if (mQueues.size() == 1
        and mQueues.begin()->second->size() == 1) {

        // First message was added for further re-sending.
        rescheduleResending();
    }
}

void ConfirmationRequiredMessagesHandler::tryProcessConfirmation(
    const ConfirmationMessage::Shared confirmationMessage)
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

        if (confirmationMessage->state() == ConfirmationMessage::ErrorShouldBeRemovedFromQueue) {
            warning() << "Contractor " << queueKey.second << " reject message "
                      << confirmationMessage->typeID() << " for equivalent " << queueKey.first;
        }

        auto ioTransaction = mCommunicatorStorageHandler->beginTransaction();
        ioTransaction->communicatorMessagesQueueHandler()->deleteRecord(
            queueKey.second,
            confirmationMessage->transactionUUID());

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        debug() << "Confirmation for message with transaction UUID " << confirmationMessage->transactionUUID() << " received. "
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

const DateTime ConfirmationRequiredMessagesHandler::closestQueueSendingTimestamp() const
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

void ConfirmationRequiredMessagesHandler::rescheduleResending()
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

void ConfirmationRequiredMessagesHandler::sendPostponedMessages() const
{
    const auto now = utc_now();

    for (const auto &contractorUUIDAndQueue : mQueues) {
        const auto kContractor = contractorUUIDAndQueue.first.second;
        const auto kQueue = contractorUUIDAndQueue.second;

        if (kQueue->nextSendingAttemptDateTime() > now) {
            // This queue's timeout is not fired up yet.
            continue;
        }

        for (const auto &transactionUUIDAndMessage : kQueue->messages()) {
            signalOutgoingMessageReady(
                make_pair(
                    kContractor,
                    transactionUUIDAndMessage.second));
        }
    }
}

void ConfirmationRequiredMessagesHandler::addMessageToStorage(
    const NodeUUID &contractorUUID,
    TransactionMessage::Shared message)
{
    auto bufferAndSize = message->serializeToBytes();
    try {
        ioTransactionUnique->communicatorMessagesQueueHandler()->saveRecord(
            contractorUUID,
            message->equivalent(),
            message->transactionUUID(),
            message->typeID(),
            bufferAndSize.first,
            bufferAndSize.second);
    } catch (IOError &e) {
        warning() << "Can't save message to storage. Details: " << e.message();
    }
}

void ConfirmationRequiredMessagesHandler::removeMessageFromStorage(
    const NodeUUID &contractorUUID,
    const SerializedEquivalent equivalent,
    Message::SerializedType messageType)
{
    try {
        ioTransactionUnique->communicatorMessagesQueueHandler()->deleteRecord(
            contractorUUID,
            equivalent,
            messageType);
    } catch (IOError &e) {
        warning() << "Can't remove message from storage. Details: " << e.message();
    }
}

void ConfirmationRequiredMessagesHandler::deserializeMessages()
{
    vector<tuple<const NodeUUID, BytesShared, Message::SerializedType>> messages;
    try {
        auto ioTransaction = mCommunicatorStorageHandler->beginTransaction();
        messages = ioTransaction->communicatorMessagesQueueHandler()->allMessages();
    } catch (IOError &e) {
        warning() << "Can't read serialized messages from storage. Details: " << e.message();
        return;
    }
    if (messages.empty()) {
        return;
    }
    info() << "Serialized messages count: " << messages.size();
    for (auto &message : messages) {
        NodeUUID contractorUUID = NodeUUID::empty();
        BytesShared messageBody;
        Message::SerializedType messageType;
        TransactionMessage::Shared sendingMessage;
        std::tie(contractorUUID, messageBody, messageType) = message;
        switch (messageType) {
            case Message::TrustLines_SetIncoming:
                sendingMessage = static_pointer_cast<TransactionMessage>(
                    make_shared<SetIncomingTrustLineMessage>(messageBody));
                break;
            case Message::TrustLines_SetIncomingInitial:
                sendingMessage = static_pointer_cast<TransactionMessage>(
                    make_shared<SetIncomingTrustLineInitialMessage>(messageBody));
                break;
            case Message::TrustLines_CloseOutgoing:
                sendingMessage = static_pointer_cast<TransactionMessage>(
                    make_shared<CloseOutgoingTrustLineMessage>(messageBody));
                break;
            case Message::GatewayNotification:
                sendingMessage = static_pointer_cast<TransactionMessage>(
                    make_shared<GatewayNotificationMessage>(messageBody));
                break;
            case Message::TrustLines_Audit:
                sendingMessage = static_pointer_cast<TransactionMessage>(
                    make_shared<AuditMessage>(messageBody));
                break;
            case Message::TrustLines_PublicKey:
                sendingMessage = static_pointer_cast<TransactionMessage>(
                    make_shared<PublicKeyMessage>(messageBody));
                break;
            case Message::TrustLines_ConflictResolver:
                sendingMessage = static_pointer_cast<TransactionMessage>(
                    make_shared<ConflictResolverMessage>(messageBody));
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
                _2,
                _3));
    }
    mDeserializationMessagesTimer->expires_from_now(
        chrono::seconds(
            kMessagesDeserializationDelayedSecondsTime));
    mDeserializationMessagesTimer->async_wait(
        boost::bind(
            &ConfirmationRequiredMessagesHandler::delayedRescheduleResendingAfterDeserialization,
            this));
}

void ConfirmationRequiredMessagesHandler::tryEnqueueMessageWithoutConnectingSignalsToSlots(
    const NodeUUID &contractorUUID,
    const TransactionMessage::Shared message)
{
    const auto queueKey = make_pair(
        message->equivalent(),
        contractorUUID);
    if (mQueues.count(queueKey) == 0) {
        auto newQueue = make_shared<ConfirmationRequiredMessagesQueue>(
            queueKey.first,
            contractorUUID);
        mQueues[queueKey] = newQueue;
    }

    ioTransactionUnique = mCommunicatorStorageHandler->beginTransactionUnique();
    if (!mQueues[queueKey]->enqueue(message)) {
        warning() << "Can't enqueue message after deserialization, invalid message type " << message->typeID();
        if (mQueues[queueKey]->size() == 0) {
            mQueues.erase(queueKey);
        }
    }
    ioTransactionUnique = nullptr;

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    debug() << "Message of type " << message->typeID() << " for equivalent " << message->equivalent()
            << " enqueued for confirmation receiving.";
#endif
}

void ConfirmationRequiredMessagesHandler::delayedRescheduleResendingAfterDeserialization()
{
    mDeserializationMessagesTimer->cancel();
    mDeserializationMessagesTimer = nullptr;
    rescheduleResending();
}

const string ConfirmationRequiredMessagesHandler::logHeader() const
noexcept
{
    return "[ConfirmationRequiredMessagesHandler]";
}