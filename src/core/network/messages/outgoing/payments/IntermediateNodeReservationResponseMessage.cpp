#include "IntermediateNodeReservationResponseMessage.h"


const Message::MessageType IntermediateNodeReservationResponseMessage::typeID() const
{
    return Message::Payments_IntermediateNodeReservationResponse;
}
