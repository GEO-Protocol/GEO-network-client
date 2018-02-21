#ifndef GEO_NETWORK_CLIENT_SETINCOMINGTRUSTLINEFROMGATEWAYMESSAGE_H
#define GEO_NETWORK_CLIENT_SETINCOMINGTRUSTLINEFROMGATEWAYMESSAGE_H

#include "SetIncomingTrustLineMessage.h"

class SetIncomingTrustLineFromGatewayMessage : public SetIncomingTrustLineMessage {

public:
    typedef shared_ptr<SetIncomingTrustLineFromGatewayMessage> Shared;

public:
    using SetIncomingTrustLineMessage::SetIncomingTrustLineMessage;

    const MessageType typeID() const
        noexcept;
};


#endif //GEO_NETWORK_CLIENT_SETINCOMINGTRUSTLINEFROMGATEWAYMESSAGE_H
