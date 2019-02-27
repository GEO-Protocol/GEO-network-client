#ifndef GEO_NETWORK_CLIENT_AUDITRULECOUNTPAYMENTS_H
#define GEO_NETWORK_CLIENT_AUDITRULECOUNTPAYMENTS_H

#include "BaseAuditRule.h"

class AuditRuleCountPayments : public BaseAuditRule {
public:
    typedef shared_ptr<AuditRuleCountPayments> Shared;

public:
    AuditRuleCountPayments(
        uint32_t countPayments);

    uint32_t countPayments() const;

private:
    uint32_t mCountPayments;
};


#endif //GEO_NETWORK_CLIENT_AUDITRULECOUNTPAYMENTS_H
