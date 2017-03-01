#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLEOUTGOINGMESSAGE_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLEOUTGOINGMESSAGE_H

#include "../../base/routing_tables/RoutingTablesMessage.h"

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
    void setPropagationStep(
        RoutingTablesMessage::PropagationStep propagationStep);

    void pushBack(
        const NodeUUID &node,
        vector<pair<const NodeUUID, const TrustLineDirection>> &table);

protected:
    RoutingTableOutgoingMessage(
        const NodeUUID &senderUUID);

    virtual const MessageType typeID() const = 0;

    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
        BytesShared buffer);

};
#endif //GEO_NETWORK_CLIENT_ROUTINGTABLEOUTGOINGMESSAGE_H
