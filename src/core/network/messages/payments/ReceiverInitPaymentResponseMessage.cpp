#include "ReceiverInitPaymentResponseMessage.h"


const Message::MessageType ReceiverInitPaymentResponseMessage::typeID() const
{
    return Message::Payments_ReceiverInitPaymentResponse;
}
