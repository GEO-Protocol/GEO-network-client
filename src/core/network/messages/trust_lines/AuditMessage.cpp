#include "AuditMessage.h"

const Message::MessageType AuditMessage::typeID() const
{
    return Message::TrustLines_Audit;
}