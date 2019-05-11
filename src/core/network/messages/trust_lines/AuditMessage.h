#ifndef GEO_NETWORK_CLIENT_AUDITMESSAGE_H
#define GEO_NETWORK_CLIENT_AUDITMESSAGE_H

#include "../base/transaction/TransactionMessage.h"
#include "../../../crypto/lamportscheme.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"

using namespace crypto;

class AuditMessage : public TransactionMessage {

public:
    typedef shared_ptr<AuditMessage> Shared;

public:
    AuditMessage(
        const SerializedEquivalent equivalent,
        ContractorID idOnSenderSide,
        const TransactionUUID &transactionUUID,
        const AuditNumber auditNumber,
        const TrustLineAmount &incomingAmount,
        const TrustLineAmount &outgoingAmount,
        const KeyNumber keyNumber,
        const lamport::Signature::Shared signature);

    AuditMessage(
        BytesShared buffer);

    const AuditNumber auditNumber() const;

    const TrustLineAmount& incomingAmount() const;

    const TrustLineAmount& outgoingAmount() const;

    const lamport::Signature::Shared signature() const;

    const KeyNumber keyNumber() const;

    const MessageType typeID() const override;

    const bool isCheckCachedResponse() const override;

    pair<BytesShared, size_t> serializeToBytes() const override;

protected:
    const size_t kOffsetToInheritedBytes() const override;

private:
    AuditNumber mAuditNumber;
    TrustLineAmount mIncomingAmount;
    TrustLineAmount mOutgoingAmount;
    uint32_t mKeyNumber;
    lamport::Signature::Shared mSignature;
};


#endif //GEO_NETWORK_CLIENT_AUDITMESSAGE_H
