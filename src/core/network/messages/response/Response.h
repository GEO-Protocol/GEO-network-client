#ifndef GEO_NETWORK_CLIENT_RESPONSE_H
#define GEO_NETWORK_CLIENT_RESPONSE_H

#include "../../../common/Types.h"

#include "../TrustLinesMessage.h"

#include <memory>
#include <utility>
#include <stdint.h>
#include <malloc.h>

using namespace std;

class Response : public TrustLinesMessage {
public:
    typedef shared_ptr<Response> Shared;

public:
    Response(
        byte* buffer);

    Response(
        NodeUUID sender,
        TransactionUUID transactionUUID,
        uint16_t code);

    uint16_t code();

    pair<ConstBytesShared, size_t> serialize();

    void deserialize(
        byte *buffer);

    const MessageTypeID typeID() const;

public:
    uint16_t mCode;
};


#endif //GEO_NETWORK_CLIENT_RESPONSE_H
