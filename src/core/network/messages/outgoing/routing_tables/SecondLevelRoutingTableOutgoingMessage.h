#ifndef GEO_NETWORK_CLIENT_SECONDLEVELROUTINGTABLEOUTGOINGMESSAGE_H
#define GEO_NETWORK_CLIENT_SECONDLEVELROUTINGTABLEOUTGOINGMESSAGE_H

#include "RoutingTableOutgoingMessage.h"

#include "../../../../common/Types.h"

#include "../../../../common/NodeUUID.h"
#include "../../../../transactions/transactions/base/TransactionUUID.h"

#include <memory>

class SecondLevelRoutingTableOutgoingMessage : public RoutingTableOutgoingMessage {
public:
    typedef shared_ptr<SecondLevelRoutingTableOutgoingMessage> Shared;

public:
    SecondLevelRoutingTableOutgoingMessage(
        NodeUUID &senderUUID,
        TrustLineUUID &trustLineUUID);

private:
    const MessageType typeID() const;

};


#endif //GEO_NETWORK_CLIENT_SECONDLEVELROUTINGTABLEOUTGOINGMESSAGE_H
