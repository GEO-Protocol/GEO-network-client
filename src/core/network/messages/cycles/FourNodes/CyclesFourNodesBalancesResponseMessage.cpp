#include "CyclesFourNodesBalancesResponseMessage.h"

const Message::MessageType CyclesFourNodesBalancesResponseMessage::typeID() const {
    return Message::MessageType::Cycles_FourNodesBalancesResponse;
}

const bool CyclesFourNodesBalancesResponseMessage::isTransactionMessage() const
    noexcept
{
    return true;
}