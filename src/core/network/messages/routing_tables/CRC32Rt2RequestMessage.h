#ifndef GEO_NETWORK_CLIENT_REQUESTROUTGHTINGTABLESCRC32REQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_REQUESTROUTGHTINGTABLESCRC32REQUESTMESSAGE_H


#include "../base/transaction/TransactionMessage.h"


class CRC32Rt2RequestMessage :
    public TransactionMessage {

public:
    typedef shared_ptr<CRC32Rt2RequestMessage> Shared;

public:
    enum CRC32SumType {
        FirstAndSecondLevel,
        FirstLevel,
        SecondLevel,
    };
public:

    CRC32Rt2RequestMessage(
        const NodeUUID &senderUIID,
        const TransactionUUID &transactionUUID,
        const CRC32SumType type);

    CRC32Rt2RequestMessage(
        BytesShared buffer);

public:
    pair<BytesShared, size_t> serializeToBytes() const
    throw (bad_alloc);

    virtual const MessageType typeID() const
    noexcept;

    const CRC32SumType CRC32Type() const;

protected:
    CRC32SumType mCRC32Type;

};

#endif //GEO_NETWORK_CLIENT_REQUESTROUTGHTINGTABLESCRC32REQUESTMESSAGE_H
