#ifndef GEO_NETWORK_CLIENT_CLOSETRUSTLINERESULT_H
#define GEO_NETWORK_CLIENT_CLOSETRUSTLINERESULT_H

#include "Result.h"
#include "../trust_lines/TrustLine.h"

class CloseTrustLineResult : public Result {
private:
    uuids::uuid mContractorUuid;
    trust_amount mAmount;

public:
    CloseTrustLineResult(Command *command,
                         const uint16_t &resultCode,
                         const string &timestampCompleted,
                         const boost::uuids::uuid &contractorUuid);

    const uint16_t &resultCode() const;

    const string &timestampExcepted() const;

    const string &timestampCompleted() const;

    const boost::uuids::uuid &contractorUuid() const;

    string serialize();
};

#endif //GEO_NETWORK_CLIENT_CLOSETRUSTLINERESULT_H
