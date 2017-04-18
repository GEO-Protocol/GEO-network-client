#include "NeighborsRequestMessage.h"


const Message::MessageType NeighborsRequestMessage::typeID () const
    noexcept
{
    return Message::RoutingTables_NeighborsRequest;
}
