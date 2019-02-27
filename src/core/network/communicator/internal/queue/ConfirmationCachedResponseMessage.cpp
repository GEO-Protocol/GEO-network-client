#include "ConfirmationCachedResponseMessage.h"

ConfirmationCachedResponseMessage::ConfirmationCachedResponseMessage(
    TransactionMessage::Shared cachedMessage,
    Message::MessageType incomingMessageTypeFilter,
    uint32_t cacheLivingTimeSeconds):
    mCachedMessage(cachedMessage),
    mIncomingMessageTypeFilter(incomingMessageTypeFilter),
    mCacheLivingTimeSeconds(cacheLivingTimeSeconds),
    mTimeCreated(utc_now())
{}

TransactionMessage::Shared ConfirmationCachedResponseMessage::getCachedMessage(
    TransactionMessage::Shared incomingMessage)
{
    if (incomingMessage->typeID() != mIncomingMessageTypeFilter) {
        return nullptr;
    }
    if (incomingMessage->typeID() == Message::TrustLines_Initial) {
        if (incomingMessage->transactionUUID() == mCachedMessage->transactionUUID()) {
            return mCachedMessage;
        }
    } else if (incomingMessage->typeID() == Message::TrustLines_PublicKeysSharingInit) {
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

bool ConfirmationCachedResponseMessage::isLegacyCache() const
{
    auto duration = Duration(0, 0, mCacheLivingTimeSeconds);
    return utc_now() - mTimeCreated > duration;
}

const DateTime ConfirmationCachedResponseMessage::legacyDateTime()
{
    return mTimeCreated + boost::posix_time::seconds(mCacheLivingTimeSeconds);
}