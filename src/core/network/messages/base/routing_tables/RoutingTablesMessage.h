#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLESMESSAGE_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLESMESSAGE_H

#include "../abstract/SenderMessage.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"
#include "../../../../common/memory/MemoryUtils.h"

#include <memory>
#include <utility>
#include <stdint.h>

class RoutingTablesMessage : public SenderMessage {
public:
    typedef shared_ptr<RoutingTablesMessage> Shared;
    typedef uint64_t RecordsCount;


protected:
    RoutingTablesMessage();

    RoutingTablesMessage(
        const NodeUUID &senderUUID);

    virtual const MessageType typeID() const = 0;

    virtual pair<BytesShared, size_t> serializeToBytes() = 0;

    virtual void deserializeFromBytes(
        BytesShared buffer) = 0;

};
#endif //GEO_NETWORK_CLIENT_ROUTINGTABLESMESSAGE_H
