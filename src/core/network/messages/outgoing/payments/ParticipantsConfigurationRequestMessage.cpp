#include "ParticipantsConfigurationRequestMessage.h"


const Message::MessageType ParticipantsConfigurationRequestMessage::typeID () const
    noexcept
{
    return Message::Payments_ParticipantsConfigurationRequest;
}
