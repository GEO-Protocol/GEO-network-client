#include "TTLProlongationRequestMessage.h"

const Message::MessageType TTLProlongationRequestMessage::typeID() const
{
    return Message::Payments_TTLProlongationRequest;
}