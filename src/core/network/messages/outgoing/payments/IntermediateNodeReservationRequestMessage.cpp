#include "IntermediateNodeReservationRequestMessage.h"


const Message::MessageType IntermediateNodeReservationRequestMessage::typeID() const
{
    return Message::Payments_IntermediateNodeReservationRequest;
}
