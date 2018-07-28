#include "ConfirmationCachedResponseMessage.h"

ConfirmationCachedResponseMessage::ConfirmationCachedResponseMessage(
    TransactionMessage::Shared cachedMessage,
    Message::MessageType incomingMessageTypeFilter):
    mCachedMessage(cachedMessage),
    mIncomingMessageTypeFilter(incomingMessageTypeFilter)
{}

TransactionMessage::Shared ConfirmationCachedResponseMessage::getCachedMessage(
    TransactionMessage::Shared incomingMessage)
{
    if (incomingMessage->typeID() == mIncomingMessageTypeFilter) {
        return mCachedMessage;
    }
    return nullptr;
}