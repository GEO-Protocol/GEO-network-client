#include "AuditRuleCountPayments.h"

AuditRuleCountPayments::AuditRuleCountPayments(
    uint32_t countPayments) :
    BaseAuditRule(BaseAuditRule::AuditRuleCountPaymentsType),
    mCountPayments(countPayments)
{}

bool AuditRuleCountPayments::check(
    TrustLine::Shared trustLine,
    IOTransaction::Shared ioTransaction)
{
    auto currentAuditNumber = ioTransaction->auditHandler()->getActualAuditNumber(
        trustLine->trustLineID());
    auto countIncomingPayments = ioTransaction->incomingPaymentReceiptHandler()->countReceiptsByNumber(
        trustLine->trustLineID(),
        currentAuditNumber);
    auto countOutgoingPayments = ioTransaction->outgoingPaymentReceiptHandler()->countReceiptsByNumber(
        trustLine->trustLineID(),
        currentAuditNumber);
    return countIncomingPayments + countOutgoingPayments >= mCountPayments;
}

void AuditRuleCountPayments::reset()
{}

uint32_t AuditRuleCountPayments::countPayments() const
{
    return mCountPayments;
}