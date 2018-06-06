#ifndef GEO_NETWORK_CLIENT_AUDITMESSAGE_H
#define GEO_NETWORK_CLIENT_AUDITMESSAGE_H

#include "../base/transaction/TransactionMessage.h"

class AuditMessage : public TransactionMessage {

public:
    typedef shared_ptr<AuditMessage> Shared;

public:
    AuditMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const uint32_t keyNumber,
        const size_t signedDataSize,
        BytesShared signedData);

    AuditMessage(
        BytesShared buffer);

    BytesShared signedData() const;

    const size_t signedDataSize() const;

    const uint32_t keyNumber() const;

    const MessageType typeID() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const;

private:
    uint32_t mKeyNumber;
    BytesShared mSignedData;
    size_t mSignedDataSize;
};


#endif //GEO_NETWORK_CLIENT_AUDITMESSAGE_H
