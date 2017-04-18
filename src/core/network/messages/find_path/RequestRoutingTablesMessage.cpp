#include "RequestRoutingTablesMessage.h"


const Message::MessageType RequestRoutingTablesMessage::typeID() const
    noexcept
{
    return Message::Paths_RequestRoutingTables;
}
