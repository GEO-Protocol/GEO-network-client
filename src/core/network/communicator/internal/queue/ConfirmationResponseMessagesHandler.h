#ifndef GEO_NETWORK_CLIENT_CONFIRMATIONRESPONSEMESSAGESHANDLER_H
#define GEO_NETWORK_CLIENT_CONFIRMATIONRESPONSEMESSAGESHANDLER_H

#include "ConfirmationCachedResponseMessage.h"
#include "../../../../logger/LoggerMixin.hpp"

#include <map>

class ConfirmationResponseMessagesHandler : LoggerMixin {

public:
    ConfirmationResponseMessagesHandler(
        Logger &logger);

    void addCachedMessage(
        const NodeUUID &contractorUUID,
        TransactionMessage::Shared cachedMessage,
        Message::MessageType incomingMessageTypeFilter);

    Message::Shared getCachedMessage(
        TransactionMessage::Shared incomingMessage);

protected:
    const string logHeader() const
    noexcept;

private:
    map<pair<SerializedEquivalent, NodeUUID>, ConfirmationCachedResponseMessage::Shared> mCachedMessages;
};


#endif //GEO_NETWORK_CLIENT_CONFIRMATIONRESPONSEMESSAGESHANDLER_H
