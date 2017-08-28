#include "ConfirmationMessage.h"


const Message::MessageType ConfirmationMessage::typeID() const
    noexcept
{
    return Message::System_Confirmation;
}
