#ifndef GEO_NETWORK_CLIENT_CONFLICTRESOLVERRESPONSEMESSAGE_H
#define GEO_NETWORK_CLIENT_CONFLICTRESOLVERRESPONSEMESSAGE_H

#include "../base/transaction/ConfirmationMessage.h"

class ConflictResolverResponseMessage : public ConfirmationMessage {

public:
    typedef shared_ptr<ConflictResolverResponseMessage> Shared;

public:
    using ConfirmationMessage::ConfirmationMessage;

protected:
    const MessageType typeID() const;
};


#endif //GEO_NETWORK_CLIENT_CONFLICTRESOLVERRESPONSEMESSAGE_H
