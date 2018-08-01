#ifndef GEO_NETWORK_CLIENT_AUDITRESPONSEMESSAGE_H
#define GEO_NETWORK_CLIENT_AUDITRESPONSEMESSAGE_H

#include "../base/transaction/ConfirmationMessage.h"
#include "../../../crypto/lamportscheme.h"

using namespace crypto;

class AuditResponseMessage : public ConfirmationMessage {

public:
    typedef shared_ptr<AuditResponseMessage> Shared;

public:
    AuditResponseMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const KeyNumber keyNumber,
        const lamport::Signature::Shared signature);

    AuditResponseMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        OperationState state);

    AuditResponseMessage(
       BytesShared buffer);

    const lamport::Signature::Shared signature() const;

    const KeyNumber keyNumber() const;

    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes() const
    throw (bad_alloc);

private:
    uint32_t mKeyNumber;
    lamport::Signature::Shared mSignature;
};


#endif //GEO_NETWORK_CLIENT_AUDITRESPONSEMESSAGE_H
