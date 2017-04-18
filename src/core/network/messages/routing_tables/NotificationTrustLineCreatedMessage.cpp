#include "NotificationTrustLineCreatedMessage.h"


const Message::MessageType NotificationTrustLineCreatedMessage::typeID() const
    noexcept
{
    return Message::RoutingTables_NotificationTrustLineCreated;
}