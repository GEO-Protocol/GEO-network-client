#include "ConfirmationResponseMessagesHandler.h"

ConfirmationResponseMessagesHandler::ConfirmationResponseMessagesHandler(
    IOService &ioService,
    Logger &logger) :
    mIOService(ioService),
    mCleaningTimer(ioService),
    LoggerMixin(logger)
{}

void ConfirmationResponseMessagesHandler::addCachedMessage(
    ContractorID contractorID,
    TransactionMessage::Shared cachedMessage,
    Message::MessageType incomingMessageTypeFilter,
    uint32_t cacheLivingTime)
{
    auto keyMap = make_pair(
        cachedMessage->equivalent(),
        contractorID);
    mCachedMessages[keyMap] = make_shared<ConfirmationCachedResponseMessage>(
        cachedMessage,
        incomingMessageTypeFilter,
        cacheLivingTime);
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    debug() << "Message " << cachedMessage->typeID() << " added for " << incomingMessageTypeFilter
            << " from contractor" << contractorID << " equivalent " << cachedMessage->equivalent();
    debug() << "mCachedMessages size " << mCachedMessages.size();
#endif
    if (mCachedMessages.size() == 1) {
        rescheduleResending();
    }
}

Message::Shared ConfirmationResponseMessagesHandler::getCachedMessage(
    TransactionMessage::Shared incomingMessage)
{
    auto keyMap = make_pair(
        incomingMessage->equivalent(),
        incomingMessage->idOnReceiverSide);

    auto confirmationCachedResponseMessage = mCachedMessages.find(keyMap);
    if (confirmationCachedResponseMessage == mCachedMessages.end()) {
        return nullptr;
    }
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    debug() << "Cache was found for " << incomingMessage->idOnReceiverSide
            << " equivalent " << incomingMessage->equivalent();
#endif
    return confirmationCachedResponseMessage->second->getCachedMessage(
        incomingMessage);
}

const DateTime ConfirmationResponseMessagesHandler::closestLegacyCacheTimestamp() const
    noexcept
{
    if (mCachedMessages.empty()) {
        return utc_now() + boost::posix_time::seconds(2);
    }

    DateTime nextClearingDateTime = mCachedMessages.begin()->second->legacyDateTime();
    for (const auto &contractorUUIDAndQueue : mCachedMessages) {
        const auto kQueueNextAttemptPlanned = contractorUUIDAndQueue.second->legacyDateTime();
        if (kQueueNextAttemptPlanned < nextClearingDateTime) {
            nextClearingDateTime = kQueueNextAttemptPlanned;
        }
    }
    return nextClearingDateTime;
}

void ConfirmationResponseMessagesHandler::rescheduleResending()
{
    if (mCachedMessages.empty()) {

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        this->debug() << "There are no cached messages present. "
                         "Cleaning would not be scheduled any more.";
#endif

        return;
    }

    const auto kCleaningTimeout = closestLegacyCacheTimestamp() - utc_now();
    mCleaningTimer.expires_from_now(chrono::microseconds(kCleaningTimeout.total_microseconds()));
    mCleaningTimer.async_wait([this] (const boost::system::error_code &e) {

        if (e == boost::asio::error::operation_aborted) {
            return;
        }

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        this->debug() << "Enqueued messages cleaning started.";
#endif

        this->clearLegacyCacheMessages();
        this->rescheduleResending();

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        this->debug() << "Enqueued messages cleaning finished.";
#endif
    });
}

void ConfirmationResponseMessagesHandler::clearLegacyCacheMessages()
{
    for (const auto &contractorIDAndCachedMessage : mCachedMessages) {
        const auto key = contractorIDAndCachedMessage.first;
        const auto kCachedMessage = contractorIDAndCachedMessage.second;

        if (kCachedMessage->isLegacyCache()) {
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
            this->debug() << "Cached message removed";
#endif
            // This cache's timeout is fired up.
            mCachedMessages.erase(key);
            return;
        }
    }
}

const string ConfirmationResponseMessagesHandler::logHeader() const
    noexcept
{
    return "[ConfirmationResponseMessagesHandler]";
}