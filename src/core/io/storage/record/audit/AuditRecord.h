#ifndef GEO_NETWORK_CLIENT_AUDITRECORD_H
#define GEO_NETWORK_CLIENT_AUDITRECORD_H

#include "../../../../common/Types.h"

class AuditRecord {
public:
    typedef shared_ptr<AuditRecord> Shared;

public:
    AuditRecord(
        AuditNumber auditNumber,
        TrustLineAmount &incomingAmount,
        TrustLineAmount &outgoingAmount,
        TrustLineBalance &balance);

    const AuditNumber auditNumber() const;

    const TrustLineAmount& incomingAmount() const;

    const TrustLineAmount& outgoingAmount() const;

    const TrustLineBalance& balance() const;

private:
    AuditNumber mAuditNumber;
    TrustLineAmount mIncomingAmount;
    TrustLineAmount mOutgoingAmount;
    TrustLineBalance mBalance;
};


#endif //GEO_NETWORK_CLIENT_AUDITRECORD_H
