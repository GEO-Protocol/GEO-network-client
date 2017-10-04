#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLERESPONCEMESSAGE_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLERESPONCEMESSAGE_H

#include "../SenderMessage.h"
#include <set>

class RoutingTableResponseMessage :
    public SenderMessage {

public:
    typedef shared_ptr<RoutingTableResponseMessage> Shared;

public:
    RoutingTableResponseMessage(
        const NodeUUID &sender,
        set<NodeUUID> neighbors
    );

    RoutingTableResponseMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes() const;

    set<NodeUUID> neighbors() const;

protected:
    set<NodeUUID> mNeighbors;
};

#endif //GEO_NETWORK_CLIENT_ROUTINGTABLERESPONCEMESSAGE_H
