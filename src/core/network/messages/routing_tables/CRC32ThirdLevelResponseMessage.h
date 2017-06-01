#ifndef GEO_NETWORK_CLIENT_CRC32THIRDLEVELRESPONSEMESSAGE_H
#define GEO_NETWORK_CLIENT_CRC32THIRDLEVELRESPONSEMESSAGE_H


#include "../base/transaction/TransactionMessage.h"


class CRC32ThirdLevelResponseMessage:
    public TransactionMessage {

public:
    typedef shared_ptr<CRC32ThirdLevelResponseMessage> Shared;

public:
    CRC32ThirdLevelResponseMessage(
        const NodeUUID &senderUIID,
        const TransactionUUID &transactionUUID,
        vector<pair<NodeUUID, uint32_t >> crc32sums);

    CRC32ThirdLevelResponseMessage(
        BytesShared buffer);

public:
    pair<BytesShared, size_t> serializeToBytes() const
    throw (bad_alloc);

    virtual const MessageType typeID() const
    noexcept;

    const vector<pair<NodeUUID, uint32_t>> crc32Sums() const;

protected:
    vector<pair<NodeUUID, uint32_t>> mCRC32Sums;
};

#endif //GEO_NETWORK_CLIENT_CRC32THIRDLEVELRESPONSEMESSAGE_H
