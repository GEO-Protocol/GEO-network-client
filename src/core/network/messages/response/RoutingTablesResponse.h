#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLESRESPONSE_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLESRESPONSE_H

#include "../base/abstract/SenderMessage.h"

#include "../../../common/Types.h"
#include "../../../common/NodeUUID.h"
#include "../../../common/memory/MemoryUtils.h"

#include "../../../common/exceptions/ConflictError.h"

#include <memory>
#include <utility>
#include <stdint.h>

class RoutingTablesResponse: public SenderMessage {
public:
    typedef shared_ptr<RoutingTablesResponse> Shared;

public:
    RoutingTablesResponse(
        const NodeUUID &sender,
        uint16_t code);

    RoutingTablesResponse(
        BytesShared buffer);

    const uint16_t code() const;

private:
    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
        BytesShared buffer);

public:
    uint16_t mCode;
};
#endif //GEO_NETWORK_CLIENT_ROUTINGTABLESRESPONSE_H
