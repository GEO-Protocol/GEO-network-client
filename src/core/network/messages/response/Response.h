#ifndef GEO_NETWORK_CLIENT_RESPONSE_H
#define GEO_NETWORK_CLIENT_RESPONSE_H

#include "../base/transaction/TransactionMessage.h"
#include "../../../common/memory/MemoryUtils.h"


using namespace std;


class Response:
    public TransactionMessage {

public:
    typedef shared_ptr<Response> Shared;

public:
    Response(
        const NodeUUID &sender,
        const TransactionUUID &transactionUUID,
        const uint16_t code);

    Response(
        BytesShared buffer);

    uint16_t code();

private:
    const bool isTransactionMessage() const
        noexcept;

    const MessageType typeID() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const
        throw(bad_alloc);

    void deserializeFromBytes(
        BytesShared buffer);

private:
    uint16_t mCode;
};
#endif //GEO_NETWORK_CLIENT_RESPONSE_H
