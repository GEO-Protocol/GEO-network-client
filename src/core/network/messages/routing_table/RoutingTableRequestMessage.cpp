#include "RoutingTableRequestMessage.h"

const Message::MessageType RoutingTableRequestMessage::typeID() const
{
    return Message::MessageType::RoutingTableRequest;
}
