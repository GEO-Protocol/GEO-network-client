#ifndef GEO_NETWORK_CLIENT_FIRSTLEVELROUTINGTABLEINCOMINGMESSAGE_H
#define GEO_NETWORK_CLIENT_FIRSTLEVELROUTINGTABLEINCOMINGMESSAGE_H

#include "RoutingTableIncomingMessage.h"

#include "../../../../common/Types.h"

#include "../../../../common/NodeUUID.h"
#include "../../../../transactions/TransactionUUID.h"

#include <memory>

class FirstLevelRoutingTableIncomingMessage : public RoutingTableIncomingMessage {
public:
    typedef shared_ptr<FirstLevelRoutingTableIncomingMessage> Shared;

public:
    FirstLevelRoutingTableIncomingMessage(
        byte *buffer);

private:
    const MessageTypeID typeID() const;

};


#endif //GEO_NETWORK_CLIENT_FIRSTLEVELROUTINGTABLEINCOMINGMESSAGE_H
