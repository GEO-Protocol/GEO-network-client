#include "PingMessage.h"

PingMessage::PingMessage(
    ContractorID idOnReceiverSide)
    noexcept:
    SenderMessage(
        0,
        idOnReceiverSide)
{}

PingMessage::PingMessage(
    BytesShared buffer)
    noexcept:
    SenderMessage(buffer)
{}

const Message::MessageType PingMessage::typeID() const
{
    return Message::General_Ping;
}