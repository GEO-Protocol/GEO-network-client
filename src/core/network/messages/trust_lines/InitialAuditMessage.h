#ifndef GEO_NETWORK_CLIENT_INITIALAUDITMESSAGE_H
#define GEO_NETWORK_CLIENT_INITIALAUDITMESSAGE_H

#include "../base/transaction/ConfirmationMessage.h"
#include "../../../crypto/lamportscheme.h"

using namespace crypto;

class InitialAuditMessage : public ConfirmationMessage {

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
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        OperationState state);

    InitialAuditMessage(
        BytesShared buffer);

    const lamport::Signature::Shared signature() const;

    const KeyNumber keyNumber() const;

    const MessageType typeID() const;

    const bool isAddToConfirmationRequiredMessagesHandler() const;

    pair<BytesShared, size_t> serializeToBytes() const
        throw (bad_alloc);

private:
    uint32_t mKeyNumber;
    lamport::Signature::Shared mSignature;
};


#endif //GEO_NETWORK_CLIENT_INITIALAUDITMESSAGE_H
