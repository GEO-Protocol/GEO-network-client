#include "PongMessage.h"

PongMessage::PongMessage(
    ContractorID idOnReceiverSide)
    noexcept:
    SenderMessage(
        0,
        idOnReceiverSide)
{}

PongMessage::PongMessage(
    BytesShared buffer)
    noexcept:
    SenderMessage(buffer)
{}

const Message::MessageType PongMessage::typeID() const
{
    return Message::General_Pong;
}