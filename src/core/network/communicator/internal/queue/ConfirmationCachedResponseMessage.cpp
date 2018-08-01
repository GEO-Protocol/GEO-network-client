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
    if (incomingMessage->typeID() != mIncomingMessageTypeFilter) {
        return nullptr;
    }
    if (incomingMessage->typeID() == Message::TrustLines_SetIncoming or
            incomingMessage->typeID() == Message::TrustLines_CloseOutgoing or
            incomingMessage->typeID() == Message::TrustLines_SetIncomingInitial) {
        if (incomingMessage->transactionUUID() == mCachedMessage->transactionUUID()) {
            return mCachedMessage;
        }

    } else if (incomingMessage->typeID() == Message::TrustLines_PublicKey) {
        auto incomingPublicKeyMessage = static_pointer_cast<PublicKeyMessage>(incomingMessage);
        auto cachedPublicKeyResponseMessage = static_pointer_cast<PublicKeyHashConfirmation>(mCachedMessage);
        if (incomingPublicKeyMessage->transactionUUID() == mCachedMessage->transactionUUID() and
                incomingPublicKeyMessage->number() == cachedPublicKeyResponseMessage->number()) {
            return mCachedMessage;
        }

    } else if (incomingMessage->typeID() == Message::TrustLines_Audit) {
        if (incomingMessage->transactionUUID() == mCachedMessage->transactionUUID()) {
            return mCachedMessage;
        }
    }
    return nullptr;
}