#include "ReceiverInitPaymentRequestMessage.h"


const Message::MessageType ReceiverInitPaymentRequestMessage::typeID() const
{
    return Message::Payments_ReceiverInitPaymentRequest;
}
