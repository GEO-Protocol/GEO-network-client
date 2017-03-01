#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLEINCOMINGMESSAGE_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLEINCOMINGMESSAGE_H

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

class RoutingTableIncomingMessage : public RoutingTablesMessage {
public:
    typedef shared_ptr<RoutingTableIncomingMessage> Shared;

protected:
    RoutingTableIncomingMessage(
        BytesShared buffer);

    virtual const MessageType typeID() const = 0;

    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
        BytesShared buffer);
};
#endif //GEO_NETWORK_CLIENT_ROUTINGTABLEINCOMINGMESSAGE_H
