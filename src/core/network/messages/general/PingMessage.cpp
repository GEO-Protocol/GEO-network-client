#include "PingMessage.h"

const Message::MessageType PingMessage::typeID() const
{
    return Message::General_Ping;
}