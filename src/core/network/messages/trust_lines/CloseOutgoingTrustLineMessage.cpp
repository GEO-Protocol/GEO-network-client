#include "CloseOutgoingTrustLineMessage.h"

const Message::MessageType CloseOutgoingTrustLineMessage::typeID() const
{
    return Message::TrustLines_CloseOutgoing;
}
