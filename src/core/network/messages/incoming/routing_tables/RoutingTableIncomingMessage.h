#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLEINCOMINGMESSAGE_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLEINCOMINGMESSAGE_H

#include "../../RoutingTablesMessage.h"

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

protected:
    RoutingTableIncomingMessage(
        byte *buffer);

    virtual const MessageTypeID typeID() const = 0;

private:
    pair<ConstBytesShared, size_t> serialize();

    void deserialize(
        byte* buffer);

protected:
    unique_ptr<map<NodeUUID, TrustLineDirection>> mRecords;

};


#endif //GEO_NETWORK_CLIENT_ROUTINGTABLEINCOMINGMESSAGE_H
