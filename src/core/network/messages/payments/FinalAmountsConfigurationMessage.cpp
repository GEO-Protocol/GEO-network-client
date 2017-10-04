#include "FinalAmountsConfigurationMessage.h"

const Message::MessageType FinalAmountsConfigurationMessage::typeID() const
{
    return Message::Payments_FinalAmountsConfiguration;
}
