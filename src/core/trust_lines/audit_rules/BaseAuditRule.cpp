#include "BaseAuditRule.h"

BaseAuditRule::BaseAuditRule(
    AuditRuleType auditRuleType) :
    mType(auditRuleType)
{}

const BaseAuditRule::AuditRuleType BaseAuditRule::auditRuleType() const
{
    return mType;
}