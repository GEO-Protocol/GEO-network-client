#ifndef GEO_NETWORK_CLIENT_REQUESTROUTGHTINGTABLESCRC32REQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_REQUESTROUTGHTINGTABLESCRC32REQUESTMESSAGE_H


#include "../SenderMessage.h"


class CRC32Rt2RequestMessage :
    public  SenderMessage{

public:

    using SenderMessage::SenderMessage;

public:
    const MessageType typeID() const{
        return Message::RoutingTables_CRC32Rt2RequestMessage;
    };
};


#endif //GEO_NETWORK_CLIENT_REQUESTROUTGHTINGTABLESCRC32REQUESTMESSAGE_H
