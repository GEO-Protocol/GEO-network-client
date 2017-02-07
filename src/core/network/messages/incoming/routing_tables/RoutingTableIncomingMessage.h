#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLEINCOMINGMESSAGE_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLEINCOMINGMESSAGE_H

#include "../../RoutingTablesMessage.hpp"

#include "../../../../common/Types.h"

#include "../../../../common/NodeUUID.h"

#include "../../../../common/exceptions/MemoryError.h"

#include <memory>
#include <utility>
#include <stdint.h>

using namespace std;

class RoutingTableIncomingMessage : public RoutingTablesMessage {
public:
    typedef shared_ptr<RoutingTableIncomingMessage> Shared;

public:
    pair<BytesShared, size_t> serializeToBytes();

protected:
    RoutingTableIncomingMessage(
        BytesShared buffer);

    virtual const MessageType typeID() const = 0;

    void deserializeFromBytes(
        BytesShared buffer);

protected:
    unique_ptr<map<NodeUUID, TrustLineDirection>> mRecords;

};


#endif //GEO_NETWORK_CLIENT_ROUTINGTABLEINCOMINGMESSAGE_H
