#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLEREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLEREQUESTMESSAGE_H

#include "../SenderMessage.h"


class RoutingTableRequestMessage :
    public SenderMessage {

public:
    typedef shared_ptr<RoutingTableRequestMessage> Shared;

public:
    using SenderMessage::SenderMessage;

    const MessageType typeID() const;
};

#endif //GEO_NETWORK_CLIENT_ROUTINGTABLEREQUESTMESSAGE_H
