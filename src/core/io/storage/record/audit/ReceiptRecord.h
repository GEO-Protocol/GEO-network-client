#ifndef GEO_NETWORK_CLIENT_RECEIPTRECORD_H
#define GEO_NETWORK_CLIENT_RECEIPTRECORD_H

#include "../../../../common/Types.h"
#include "../../../../crypto/lamportkeys.h"
#include "../../../../crypto/lamportscheme.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../transactions/transactions/base/TransactionUUID.h"

using namespace crypto;

class ReceiptRecord {

public:
    typedef shared_ptr<ReceiptRecord> Shared;

public:
    ReceiptRecord(
        const AuditNumber auditNumber,
        const TransactionUUID &transactionUUID,
        const TrustLineAmount &amount,
        const lamport::KeyHash::Shared keyHash,
        const lamport::Signature::Shared);

    ReceiptRecord(
        byte* buffer);

    const AuditNumber auditNumber() const;

    const TransactionUUID& transactionUUID() const;

    const TrustLineAmount& amount() const;

    const lamport::KeyHash::Shared keyHash() const;

    const lamport::Signature::Shared signature() const;

    BytesShared serializeToBytes();

    static const size_t recordSize();

    friend bool operator== (
        const ReceiptRecord::Shared record1,
        const ReceiptRecord::Shared record2);

private:
    AuditNumber mAuditNumber;
    TransactionUUID mTransactionUUID;
    TrustLineAmount mAmount;
    lamport::KeyHash::Shared mKeyHash;
    lamport::Signature::Shared mSignature;
};


#endif //GEO_NETWORK_CLIENT_RECEIPTRECORD_H
