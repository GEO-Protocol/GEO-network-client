#ifndef GEO_NETWORK_CLIENT_PAYMENTRESULT_H
#define GEO_NETWORK_CLIENT_PAYMENTRESULT_H

#include <boost/lexical_cast.hpp>
#include "Result.h"
#include "../trust_lines/TrustLine.h"

class PaymentResult : public Result {
private:
    uuids::uuid mContractorUuid;
    trust_amount mAmount;

public:
    PaymentResult(Command *command,
                  const uint16_t &resultCode,
                  const string &timestampCompleted,
                  const boost::uuids::uuid &contractorUuid,
                  const trust_amount amount);

    const uint16_t &resultCode() const;

    const string &timestampExcepted() const;

    const string &timestampCompleted() const;

    const boost::uuids::uuid &contractorUuid() const;

    const trust_amount &amount() const;

    string serialize();
};

#endif //GEO_NETWORK_CLIENT_PAYMENTRESULT_H
