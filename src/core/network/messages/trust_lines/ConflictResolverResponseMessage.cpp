#include "ConflictResolverResponseMessage.h"

const Message::MessageType ConflictResolverResponseMessage::typeID() const
{
    return Message::TrustLines_ConflictResolverConfirmation;
}
