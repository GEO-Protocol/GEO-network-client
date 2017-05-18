#include "IntermediateNodeCycleReservationRequestMessage.h"

const Message::MessageType IntermediateNodeCycleReservationRequestMessage::typeID() const
{
    return Message::Payments_IntermediateNodeCycleReservationRequest;
}