#include "FinalPathConfigurationMessage.h"

const Message::MessageType FinalPathConfigurationMessage::typeID() const
{
    return Message::Payments_FinalPathConfiguration;
}