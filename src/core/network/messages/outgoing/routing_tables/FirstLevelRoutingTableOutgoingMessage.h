#ifndef GEO_NETWORK_CLIENT_FIRSTLEVELROUTINGTABLEOUTGOINGMESSAGE_H
#define GEO_NETWORK_CLIENT_FIRSTLEVELROUTINGTABLEOUTGOINGMESSAGE_H

#include "RoutingTableOutgoingMessage.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"

#include <memory>

class FirstLevelRoutingTableOutgoingMessage: public RoutingTableOutgoingMessage {
public:
    typedef shared_ptr<FirstLevelRoutingTableOutgoingMessage> Shared;

public:
    FirstLevelRoutingTableOutgoingMessage(
        NodeUUID &senderUUID);

private:
    const MessageType typeID() const;
};

#endif //GEO_NETWORK_CLIENT_FIRSTLEVELROUTINGTABLEOUTGOINGMESSAGE_H
