#include "NotificationTrustLineCreatedMessage.h"


const MessageType NotificationTrustLineCreatedMessage::typeID() const
    noexcept
{
    return Message::RoutingTables_NotificationTrustLineCreated;
}