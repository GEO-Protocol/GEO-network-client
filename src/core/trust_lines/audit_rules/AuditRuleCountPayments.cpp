#include "AuditRuleCountPayments.h"

AuditRuleCountPayments::AuditRuleCountPayments(
    uint32_t countPayments) :
    BaseAuditRule(BaseAuditRule::AuditRuleCountPaymentsType),
    mCountPayments(countPayments)
{}

uint32_t AuditRuleCountPayments::countPayments() const
{
    return mCountPayments;
}