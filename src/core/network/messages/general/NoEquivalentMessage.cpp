#include "NoEquivalentMessage.h"

const Message::MessageType NoEquivalentMessage::typeID() const
{
    return Message::MessageType::General_NoEquivalent;
}