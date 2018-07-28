#include "ConfirmationResponseMessagesHandler.h"

ConfirmationResponseMessagesHandler::ConfirmationResponseMessagesHandler(
    Logger &logger) :
    LoggerMixin(logger)
{}

void ConfirmationResponseMessagesHandler::addCachedMessage(
    const NodeUUID &contractorUUID,
    TransactionMessage::Shared cachedMessage,
    Message::MessageType incomingMessageTypeFilter)
{
    auto keyMap = make_pair(
        cachedMessage->equivalent(),
        contractorUUID);
    mCachedMessages[keyMap] = make_shared<ConfirmationCachedResponseMessage>(
        cachedMessage,
        incomingMessageTypeFilter);
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    debug() << "Messages added for " << contractorUUID << " equivalent " << cachedMessage->equivalent();
#endif
}

Message::Shared ConfirmationResponseMessagesHandler::getCachedMessage(
    TransactionMessage::Shared incomingMessage)
{
    auto keyMap = make_pair(
        incomingMessage->equivalent(),
        incomingMessage->senderUUID);

    auto confirmationCachedResponseMessage = mCachedMessages.find(keyMap);
    if (confirmationCachedResponseMessage == mCachedMessages.end()) {
        return nullptr;
    }
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    debug() << "Cached response was found for " << incomingMessage->senderUUID
            << " equivalent " << incomingMessage->equivalent();
#endif
    return confirmationCachedResponseMessage->second->getCachedMessage(
        incomingMessage);
}

const string ConfirmationResponseMessagesHandler::logHeader() const
noexcept
{
    return "[ConfirmationResponseMessagesHandler]";
}