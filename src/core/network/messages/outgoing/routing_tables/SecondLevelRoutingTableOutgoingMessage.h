#ifndef GEO_NETWORK_CLIENT_SECONDLEVELROUTINGTABLEOUTGOINGMESSAGE_H
#define GEO_NETWORK_CLIENT_SECONDLEVELROUTINGTABLEOUTGOINGMESSAGE_H

#include "RoutingTableOutgoingMessage.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"

#include <memory>

class SecondLevelRoutingTableOutgoingMessage : public RoutingTableOutgoingMessage {
public:
    typedef shared_ptr<SecondLevelRoutingTableOutgoingMessage> Shared;

public:
    SecondLevelRoutingTableOutgoingMessage(
        const NodeUUID &senderUUID);

private:
    const MessageType typeID() const;

};
#endif //GEO_NETWORK_CLIENT_SECONDLEVELROUTINGTABLEOUTGOINGMESSAGE_H
