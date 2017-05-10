#include "DebugMessage.h"


DebugMessage::DebugMessage()
    noexcept :
    TransactionMessage (
        NodeUUID::empty(),
        TransactionUUID::empty())
{}

DebugMessage::DebugMessage(
    BytesShared bytes) :
    TransactionMessage (bytes)
{}

const Message::MessageType DebugMessage::typeID() const
{
    return Debug;
}
