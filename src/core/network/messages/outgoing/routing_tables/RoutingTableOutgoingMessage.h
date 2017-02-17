#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLEOUTGOINGMESSAGE_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLEOUTGOINGMESSAGE_H

#include "../../RoutingTablesMessage.hpp"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"
#include "../../../../common/memory/MemoryUtils.h"

#include "../../../../common/exceptions/MemoryError.h"

#include <map>
#include <vector>
#include <memory>
#include <utility>
#include <stdint.h>

using namespace std;

class RoutingTableOutgoingMessage : public RoutingTablesMessage {
public:
    typedef shared_ptr<RoutingTableOutgoingMessage> Shared;

public:
    virtual const MessageType typeID() const = 0;

    void pushBack(
        const NodeUUID &node,
        vector<pair<NodeUUID, TrustLineDirection>> &table);

    pair<BytesShared, size_t> serializeToBytes();

protected:
    RoutingTableOutgoingMessage(
        NodeUUID &senderUUID);

    void deserializeFromBytes(
        BytesShared buffer);

protected:
    map<const NodeUUID, vector<pair<NodeUUID, TrustLineDirection>>> mRecords;
};


#endif //GEO_NETWORK_CLIENT_ROUTINGTABLEOUTGOINGMESSAGE_H
