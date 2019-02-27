#ifndef GEO_NETWORK_CLIENT_AUDITRULEBOUNDARYOVERFLOWED_H
#define GEO_NETWORK_CLIENT_AUDITRULEBOUNDARYOVERFLOWED_H

#include "BaseAuditRule.h"

class AuditRuleBoundaryOverflowed : public BaseAuditRule {
public:
    typedef shared_ptr<AuditRuleBoundaryOverflowed> Shared;

public:
    AuditRuleBoundaryOverflowed();

};


#endif //GEO_NETWORK_CLIENT_AUDITRULEBOUNDARYOVERFLOWED_H
