#include "SetIncomingTrustLineFromGatewayMessage.h"

const Message::MessageType SetIncomingTrustLineFromGatewayMessage::typeID() const
    noexcept
{
    return Message::TrustLines_SetIncomingFromGateway;
}
