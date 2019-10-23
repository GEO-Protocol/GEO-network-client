#include "PingMessage.h"

PingMessage::PingMessage(
    ContractorID idOnReceiverSide):
    SenderMessage(
        0,
        idOnReceiverSide)
{}

PingMessage::PingMessage(
    BytesShared buffer):
    SenderMessage(buffer)
{}

const Message::MessageType PingMessage::typeID() const
{
    return Message::General_Ping;
}