#include "InitiateTotalBalancesMessage.h"

const Message::MessageType InitiateTotalBalancesMessage::typeID() const
    noexcept
{
    return Message::TotalBalance_Request;
}

