#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLESRESPONSE_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLESRESPONSE_H

#include "../Message.hpp"

#include "../../../common/Types.h"
#include "../../../common/NodeUUID.h"
#include "../../../common/memory/MemoryUtils.h"

#include "../../../common/exceptions/ConflictError.h"

#include <memory>
#include <utility>
#include <stdint.h>

class RoutingTablesResponse: public Message {
public:
    typedef shared_ptr<RoutingTablesResponse> Shared;

public:
    RoutingTablesResponse(
        BytesShared buffer);

    RoutingTablesResponse(
        NodeUUID &sender,
        NodeUUID &contractor,
        uint16_t code);

    const MessageType typeID() const;

    const NodeUUID &contractorUUID() const;

    const TransactionUUID &transactionUUID() const;

    const uint16_t code() const;

    pair<BytesShared, size_t> serializeToBytes();

private:
    void deserializeFromBytes(
        BytesShared buffer);

public:
    NodeUUID mContractorUUID;
    uint16_t mCode;

};


#endif //GEO_NETWORK_CLIENT_ROUTINGTABLESRESPONSE_H
