#ifndef GEO_NETWORK_CLIENT_SECONDLEVELROUTINGTABLEINCOMINGMESSAGE_H
#define GEO_NETWORK_CLIENT_SECONDLEVELROUTINGTABLEINCOMINGMESSAGE_H

#include "RoutingTableIncomingMessage.h"

#include "../../../../common/Types.h"

#include <memory>

class SecondLevelRoutingTableIncomingMessage : public RoutingTableIncomingMessage {
public:
    typedef shared_ptr<SecondLevelRoutingTableIncomingMessage> Shared;

public:
    SecondLevelRoutingTableIncomingMessage(
        BytesShared buffer);

private:
    const MessageType typeID() const;

};
#endif //GEO_NETWORK_CLIENT_SECONDLEVELROUTINGTABLEINCOMINGMESSAGE_H
