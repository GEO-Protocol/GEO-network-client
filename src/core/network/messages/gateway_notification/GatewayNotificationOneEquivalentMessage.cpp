#include "GatewayNotificationOneEquivalentMessage.h"

const Message::MessageType GatewayNotificationOneEquivalentMessage::typeID() const
{
    return Message::GatewayNotificationOneEquivalent;
}

const bool GatewayNotificationOneEquivalentMessage::isAddToConfirmationRequiredMessagesHandler() const
{
    return true;
}
