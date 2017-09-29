#include "GatewayNotificationMessage.h"

const Message::MessageType GatewayNotificationMessage::typeID() const
{
    return Message::GatewayNotification;
}