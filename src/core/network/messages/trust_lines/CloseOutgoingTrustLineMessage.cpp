#include "CloseOutgoingTrustLineMessage.h"

const Message::MessageType CloseOutgoingTrustLineMessage::typeID() const
{
    return Message::TrustLines_CloseOutgoing;
}

const bool CloseOutgoingTrustLineMessage::isAddToConfirmationRequiredMessagesHandler() const
{
    return true;
}

const bool CloseOutgoingTrustLineMessage::isCheckCachedResponse() const
{
    return true;
}
