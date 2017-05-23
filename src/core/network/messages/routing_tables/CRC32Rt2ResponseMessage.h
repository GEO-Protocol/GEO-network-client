#ifndef GEO_NETWORK_CLIENT_CRC32RT2RESPONSETMESSAGE_H
#define GEO_NETWORK_CLIENT_CRC32RT2RESPONSETMESSAGE_H

#include "../base/transaction/TransactionMessage.h"


class CRC32Rt2ResponseMessage :
    public TransactionMessage {

public:
    typedef shared_ptr<CRC32Rt2ResponseMessage> Shared;

public:

    CRC32Rt2ResponseMessage(
        const NodeUUID& nodeUUID,
        const TransactionUUID &transactionUUID,
        uint32_t& crc32rt2sum);

    CRC32Rt2ResponseMessage(
        BytesShared buffer);

    pair<BytesShared, size_t> serializeToBytes() const
    throw(bad_alloc);

    uint32_t crc32Rt2Sum() const;

public:
    const MessageType typeID() const
    noexcept;

protected:
    uint32_t mCrc32Rt2Sum;

};

#endif //GEO_NETWORK_CLIENT_CRC32RT2RESPONSETMESSAGE_H
