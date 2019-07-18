#ifndef GEO_NETWORK_CLIENT_AUDITRULEBOUNDARYOVERFLOWED_H
#define GEO_NETWORK_CLIENT_AUDITRULEBOUNDARYOVERFLOWED_H

#include "BaseAuditRule.h"

class AuditRuleBoundaryOverflowed : public BaseAuditRule {
public:
    typedef shared_ptr<AuditRuleBoundaryOverflowed> Shared;

public:
    AuditRuleBoundaryOverflowed();

    bool check(
        TrustLine::Shared trustLine,
        IOTransaction::Shared ioTransaction=nullptr) override;

    void reset() override;
};


#endif //GEO_NETWORK_CLIENT_AUDITRULEBOUNDARYOVERFLOWED_H
