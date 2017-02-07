#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLEOUTGOINGMESSAGE_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLEOUTGOINGMESSAGE_H

#include "../../RoutingTablesMessage.hpp"

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"

#include "../../../../common/NodeUUID.h"
#include "../../../../trust_lines/TrustLineUUID.h"

#include "../../../../common/exceptions/MemoryError.h"

#include <memory>
#include <utility>
#include <stdint.h>

using namespace std;

class RoutingTableOutgoingMessage : public RoutingTablesMessage {
public:
    typedef shared_ptr<RoutingTableOutgoingMessage> Shared;

public:
    void pushBack(
        NodeUUID &neighbor,
        TrustLineDirection direction);

    pair<BytesShared, size_t> serializeToBytes();

protected:
    RoutingTableOutgoingMessage(
        NodeUUID &senderUUID,
        NodeUUID &contractorUUID,
        TrustLineUUID &trustLineUUID);

    virtual const MessageType typeID() const = 0;

    void deserializeFromBytes(
        BytesShared buffer);

protected:
    unique_ptr<map<NodeUUID, TrustLineDirection>> mRecords;
};


#endif //GEO_NETWORK_CLIENT_ROUTINGTABLEOUTGOINGMESSAGE_H
