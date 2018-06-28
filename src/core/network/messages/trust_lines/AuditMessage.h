#ifndef GEO_NETWORK_CLIENT_AUDITMESSAGE_H
#define GEO_NETWORK_CLIENT_AUDITMESSAGE_H

#include "InitialAuditMessage.h"

class AuditMessage : public InitialAuditMessage {

public:
    typedef shared_ptr<InitialAuditMessage> Shared;

public:
    using InitialAuditMessage::InitialAuditMessage;

public:
    const MessageType typeID() const;
};


#endif //GEO_NETWORK_CLIENT_AUDITMESSAGE_H
