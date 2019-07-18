#include "AuditRuleBoundaryOverflowed.h"

AuditRuleBoundaryOverflowed::AuditRuleBoundaryOverflowed() :
    BaseAuditRule(BaseAuditRule::AuditRuleTrustLineAmountBoundaryType)
{}

bool AuditRuleBoundaryOverflowed::check(
    TrustLine::Shared trustLine,
    IOTransaction::Shared ioTransaction)
{
    return trustLine->isTrustLineOverflowed();
}

void AuditRuleBoundaryOverflowed::reset()
{}