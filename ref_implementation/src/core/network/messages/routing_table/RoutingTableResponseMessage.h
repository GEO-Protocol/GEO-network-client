/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

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
