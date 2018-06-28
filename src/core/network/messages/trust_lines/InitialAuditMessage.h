#ifndef GEO_NETWORK_CLIENT_INITIALAUDITMESSAGE_H
#define GEO_NETWORK_CLIENT_INITIALAUDITMESSAGE_H

#include "../base/transaction/TransactionMessage.h"
#include "../../../crypto/lamportscheme.h"

using namespace crypto;

class InitialAuditMessage : public TransactionMessage {

public:
    typedef shared_ptr<InitialAuditMessage> Shared;

public:
    InitialAuditMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const KeyNumber keyNumber,
        const lamport::Signature::Shared signature);

    InitialAuditMessage(
        BytesShared buffer);

    const lamport::Signature::Shared signature() const;

    const KeyNumber keyNumber() const;

    const MessageType typeID() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const;

private:
    uint32_t mKeyNumber;
    lamport::Signature::Shared mSignature;
};


#endif //GEO_NETWORK_CLIENT_INITIALAUDITMESSAGE_H
