#include "PongMessage.h"

const Message::MessageType PongMessage::typeID() const
{
    return Message::General_Pong;
}