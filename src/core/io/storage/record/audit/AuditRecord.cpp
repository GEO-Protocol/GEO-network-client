#include "AuditRecord.h"

AuditRecord::AuditRecord(
    AuditNumber auditNumber,
    TrustLineAmount &incomingAmount,
    TrustLineAmount &outgoingAmount,
    TrustLineBalance &balance) :

    mAuditNumber(auditNumber),
    mIncomingAmount(incomingAmount),
    mOutgoingAmount(outgoingAmount),
    mBalance(balance)
{}

const AuditNumber AuditRecord::auditNumber() const
{
    return mAuditNumber;
}

const TrustLineAmount& AuditRecord::incomingAmount() const
{
    return mIncomingAmount;
}

const TrustLineAmount& AuditRecord::outgoingAmount() const
{
    return mOutgoingAmount;
}

const TrustLineBalance& AuditRecord::balance() const
{
    return mBalance;
}