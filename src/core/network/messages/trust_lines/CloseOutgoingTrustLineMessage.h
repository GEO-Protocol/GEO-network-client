#ifndef GEO_NETWORK_CLIENT_CLOSEOUTGOINGTRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_CLOSEOUTGOINGTRUSTLINEMESSAGE_H

#include "AuditMessage.h"

class CloseOutgoingTrustLineMessage : public AuditMessage {

public:
    typedef shared_ptr<CloseOutgoingTrustLineMessage> Shared;
    typedef shared_ptr<const CloseOutgoingTrustLineMessage> ConstShared;

public:
    using AuditMessage::AuditMessage;

    const MessageType typeID() const;
};


#endif //GEO_NETWORK_CLIENT_CLOSEOUTGOINGTRUSTLINEMESSAGE_H
