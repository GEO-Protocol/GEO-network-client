#ifndef GEO_NETWORK_CLIENT_CONFIRMATIONCACHEDRESPONSEMESSAGE_H
#define GEO_NETWORK_CLIENT_CONFIRMATIONCACHEDRESPONSEMESSAGE_H

#include "../../../messages/base/transaction/TransactionMessage.h"
#include "../../../messages/trust_lines/PublicKeyMessage.h"
#include "../../../messages/trust_lines/PublicKeysSharingInitMessage.h"
#include "../../../messages/trust_lines/PublicKeyHashConfirmation.h"

#include "../../../../common/time/TimeUtils.h"

class ConfirmationCachedResponseMessage {

public:
    typedef shared_ptr<ConfirmationCachedResponseMessage> Shared;

public:
    ConfirmationCachedResponseMessage(
        TransactionMessage::Shared cachedMessage,
        Message::MessageType incomingMessageTypeFilter,
        uint32_t cacheLivingTimeSeconds);

    TransactionMessage::Shared getCachedMessage(
        TransactionMessage::Shared incomingMessage);

    bool isLegacyCache() const;

    /**
     * @returns date time when cache will be legacy.
     */
    const DateTime legacyDateTime();

private:
    TransactionMessage::Shared mCachedMessage;
    Message::MessageType mIncomingMessageTypeFilter;
    uint32_t mCacheLivingTimeSeconds;
    DateTime mTimeCreated;
};


#endif //GEO_NETWORK_CLIENT_CONFIRMATIONCACHEDRESPONSEMESSAGE_H
