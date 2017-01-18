#ifndef GEO_NETWORK_CLIENT_RESPONSE_H
#define GEO_NETWORK_CLIENT_RESPONSE_H

#include "../../../common/Types.h"

#include "../Message.h"

#include <memory>

using namespace std;

class Response : public Message {
public:
    typedef shared_ptr<Response> Shared;

    Response(
        byte* buffer);

    Response(
        NodeUUID sender,
        TransactionUUID transactionUUID,
        uint16_t code);

    pair<ConstBytesShared, size_t> serialize();

    void deserialize(
        byte *buffer);

    const MessageTypeID typeID() const;

public:
    uint16_t mCode;
};


#endif //GEO_NETWORK_CLIENT_RESPONSE_H
