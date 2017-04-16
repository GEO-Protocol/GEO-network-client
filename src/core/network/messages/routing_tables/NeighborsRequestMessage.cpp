#include "NeighborsRequestMessage.h"


const Message::MessageType NeighborsRequestMessage::typeID () const
{
    return Message::RoutingTables_NeighborsRequest;
}
