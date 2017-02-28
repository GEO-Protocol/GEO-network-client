#include "ReserveBalanceRequestMessage.h"


const Message::MessageType ReserveBalanceRequestMessage::typeID() const
{
    return Message::Payments_ReserveBalanceRequest;
}
