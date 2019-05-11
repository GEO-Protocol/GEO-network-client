#include "PongMessage.h"

PongMessage::PongMessage(
    ContractorID idOnReceiverSide):
    SenderMessage(
        0,
        idOnReceiverSide)
{}

PongMessage::PongMessage(
    BytesShared buffer):
    SenderMessage(buffer)
{}

const Message::MessageType PongMessage::typeID() const
{
    return Message::General_Pong;
}