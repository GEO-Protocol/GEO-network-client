#ifndef GEO_NETWORK_CLIENT_AUDITRECORD_H
#define GEO_NETWORK_CLIENT_AUDITRECORD_H

#include "../../../../common/Types.h"
#include "../../../../crypto/lamportkeys.h"
#include "../../../../crypto/lamportscheme.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"
#include "../../../../common/memory/MemoryUtils.h"

using namespace crypto;

class AuditRecord {
public:
    typedef shared_ptr<AuditRecord> Shared;

public:
    AuditRecord(
        AuditNumber auditNumber,
        TrustLineAmount &incomingAmount,
        TrustLineAmount &outgoingAmount,
        TrustLineBalance &balance);

    AuditRecord(
        AuditNumber auditNumber,
        TrustLineAmount &incomingAmount,
        TrustLineAmount &outgoingAmount,
        TrustLineBalance &balance,
        lamport::KeyHash::Shared ownKeyHash,
        lamport::Signature::Shared ownSignature,
        lamport::KeyHash::Shared contractorKeyHash,
        lamport::Signature::Shared contractorSignature);

    AuditRecord(
        byte* buffer);

    const AuditNumber auditNumber() const;

    const TrustLineAmount& incomingAmount() const;

    const TrustLineAmount& outgoingAmount() const;

    const TrustLineBalance& balance() const;

    const lamport::KeyHash::Shared ownKeyHash() const;

    const lamport::Signature::Shared ownSignature() const;

    const lamport::KeyHash::Shared contractorKeyHash() const;

    const lamport::Signature::Shared contractorSignature() const;

    BytesShared serializeToBytes();

    BytesShared serializeToCheckSignatureByInitiator();

    BytesShared serializeToCheckSignatureByContractor();

    static const size_t recordSize();

    static const size_t recordSizeForSignatureChecking();

private:
    AuditNumber mAuditNumber;
    TrustLineAmount mIncomingAmount;
    TrustLineAmount mOutgoingAmount;
    TrustLineBalance mBalance;
    lamport::KeyHash::Shared mOwnKeyHash;
    lamport::Signature::Shared mOwnSignature;
    lamport::KeyHash::Shared mContractorKeyHash;
    lamport::Signature::Shared mContractorSignature;
};


#endif //GEO_NETWORK_CLIENT_AUDITRECORD_H
