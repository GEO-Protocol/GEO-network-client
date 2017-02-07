#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLEOUTGOINGMESSAGE_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLEOUTGOINGMESSAGE_H

#include "../../RoutingTablesMessage.h"

#include "../../../../common/Types.h"

#include "../../../../common/NodeUUID.h"
#include "../../../../trust_lines/TrustLineUUID.h"

#include "../../../../common/exceptions/MemoryError.h"

#include <memory>
#include <utility>
#include <stdint.h>
#include <malloc.h>

using namespace std;

class RoutingTableOutgoingMessage : public RoutingTablesMessage {
public:
    typedef shared_ptr<RoutingTableOutgoingMessage> Shared;

public:
    void pushBack(
        NodeUUID &neighbor,
        TrustLineDirection direction);

protected:
    RoutingTableOutgoingMessage(
        NodeUUID &senderUUID,
        NodeUUID &contractorUUID,
        TrustLineUUID &trustLineUUID);

    virtual const MessageTypeID typeID() const = 0;

private:
    pair<ConstBytesShared, size_t> serialize();

    void deserialize(
        byte *buffer);

protected:
    unique_ptr<map<NodeUUID, TrustLineDirection>> mRecords;
};


#endif //GEO_NETWORK_CLIENT_ROUTINGTABLEOUTGOINGMESSAGE_H
