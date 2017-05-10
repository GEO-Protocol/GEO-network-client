#include "TTLPolongationMessage.h"

const Message::MessageType TTLPolongationMessage::typeID() const
{
    return Message::Payments_TTLProlongation;
}