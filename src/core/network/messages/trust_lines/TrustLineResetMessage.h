#ifndef GEO_NETWORK_CLIENT_TRUSTLINERESETMESSAGE_H
#define GEO_NETWORK_CLIENT_TRUSTLINERESETMESSAGE_H

#include "../base/transaction/TransactionMessage.h"

class TrustLineResetMessage : public TransactionMessage {

public:
    typedef shared_ptr<TrustLineResetMessage> Shared;

public:
    TrustLineResetMessage(
        const SerializedEquivalent equivalent,
        Contractor::Shared contractor,
        const TransactionUUID &transactionUUID,
        const AuditNumber auditNumber,
        const TrustLineAmount &incomingAmount,
        const TrustLineAmount &outgoingAmount,
        const TrustLineBalance &balance);

    TrustLineResetMessage(
        BytesShared buffer);

    const AuditNumber auditNumber() const;

    const TrustLineAmount& incomingAmount() const;

    const TrustLineAmount& outgoingAmount() const;

    const TrustLineBalance& balance() const;

    const MessageType typeID() const override;

    const bool isCheckCachedResponse() const override;

    pair<BytesShared, size_t> serializeToBytes() const override;

private:
    AuditNumber mAuditNumber;
    TrustLineAmount mIncomingAmount;
    TrustLineAmount mOutgoingAmount;
    TrustLineBalance mBalance;
};

#endif //GEO_NETWORK_CLIENT_TRUSTLINERESETMESSAGE_H
