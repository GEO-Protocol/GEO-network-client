#ifndef GEO_NETWORK_CLIENT_RESPONSE_H
#define GEO_NETWORK_CLIENT_RESPONSE_H

#include "../TrustLinesMessage.hpp"

#include "../../../common/Types.h"
#include "../../../common/memory/MemoryUtils.h"

#include <memory>
#include <utility>
#include <stdint.h>

using namespace std;

class Response : public TrustLinesMessage {
public:
    typedef shared_ptr<Response> Shared;

public:
    Response(
        BytesShared buffer);

    Response(
        NodeUUID &sender,
        TransactionUUID &transactionUUID,
        uint16_t code);

    const MessageType typeID() const;

    uint16_t code();

    pair<BytesShared, size_t> serializeToBytes();

private:
    void deserializeFromBytes(
        BytesShared buffer);

public:
    uint16_t mCode;
};


#endif //GEO_NETWORK_CLIENT_RESPONSE_H
