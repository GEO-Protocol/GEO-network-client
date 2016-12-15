#ifndef GEO_NETWORK_CLIENT_TOTALBALANCERESULT_H
#define GEO_NETWORK_CLIENT_TOTALBALANCERESULT_H

#include "Result.h"

class TotalBalanceResult : public Result {
private:
    trust_amount mTotalIncomingTrust;
    trust_amount mTotalIncomingTrustUsed;
    trust_amount mTotalOutgoingTrust;
    trust_amount mTotalOutgoingTrustUsed;

public:
    TotalBalanceResult(Command *command,
                       const uint16_t &resultCode,
                       const string &timestampCompleted,
                       const trust_amount &totalIncomingTrust,
                       const trust_amount &totalIncomingTrustUsed,
                       const trust_amount &totalOutgoingTrust,
                       const trust_amount &totalOutgoingTrustUsed);

    const uuids::uuid &commandUUID() const;

    const string &id() const;

    const uint16_t &resultCode() const;

    const string &timestampExcepted() const;

    const string &timestampCompleted() const;

    const trust_amount &totalIncomingTrust() const;

    const trust_amount &totalIncomingTrustUsed() const;

    const trust_amount &totalOutgoingTrust() const;

    const trust_amount &totalOutgoingTrustUsed() const;

    string serialize();
};

#endif //GEO_NETWORK_CLIENT_TOTALBALANCERESULT_H
