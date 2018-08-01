#ifndef GEO_NETWORK_CLIENT_CONFIRMATIONCACHEDRESPONSEMESSAGE_H
#define GEO_NETWORK_CLIENT_CONFIRMATIONCACHEDRESPONSEMESSAGE_H

#include "../../../messages/base/transaction/TransactionMessage.h"
#include "../../../messages/trust_lines/PublicKeyMessage.h"
#include "../../../messages/trust_lines/PublicKeyHashConfirmation.h"

class ConfirmationCachedResponseMessage {

public:
    typedef shared_ptr<ConfirmationCachedResponseMessage> Shared;

public:
    ConfirmationCachedResponseMessage(
        TransactionMessage::Shared cachedMessage,
        Message::MessageType incomingMessageTypeFilter);

    TransactionMessage::Shared getCachedMessage(
        TransactionMessage::Shared incomingMessage);

private:
    TransactionMessage::Shared mCachedMessage;
    Message::MessageType mIncomingMessageTypeFilter;
};


#endif //GEO_NETWORK_CLIENT_CONFIRMATIONCACHEDRESPONSEMESSAGE_H
