#include "DebugMessage.h"


DebugMessage::DebugMessage() :
    TransactionMessage (
        NodeUUID::empty(),
        TransactionUUID::empty())
{}

const Message::MessageType DebugMessage::typeID() const
{
    return Debug;
}
