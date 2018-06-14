#ifndef GEO_NETWORK_CLIENT_AUDITMESSAGE_H
#define GEO_NETWORK_CLIENT_AUDITMESSAGE_H

#include "../base/transaction/TransactionMessage.h"
#include "../../../crypto/lamportscheme.h"

using namespace crypto;

class AuditMessage : public TransactionMessage {

public:
    typedef shared_ptr<AuditMessage> Shared;

public:
    AuditMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const KeyNumber keyNumber,
        const lamport::Signature::Shared signedData);

    AuditMessage(
        BytesShared buffer);

    const lamport::Signature::Shared signedData() const;

    const KeyNumber keyNumber() const;

    const MessageType typeID() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const;

private:
    uint32_t mKeyNumber;
    lamport::Signature::Shared mSignedData;
};


#endif //GEO_NETWORK_CLIENT_AUDITMESSAGE_H
