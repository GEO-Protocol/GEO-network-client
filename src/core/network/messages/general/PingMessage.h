#ifndef GEO_NETWORK_CLIENT_PINGMESSAGE_H
#define GEO_NETWORK_CLIENT_PINGMESSAGE_H

#include "../SenderMessage.h"

class PingMessage : public SenderMessage {

public:
    typedef shared_ptr<PingMessage> Shared;

public:
    using SenderMessage::SenderMessage;

    const MessageType typeID() const;
};


#endif //GEO_NETWORK_CLIENT_PINGMESSAGE_H
