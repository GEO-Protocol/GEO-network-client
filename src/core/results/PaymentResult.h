#ifndef GEO_NETWORK_CLIENT_PAYMENTRESULT_H
#define GEO_NETWORK_CLIENT_PAYMENTRESULT_H

#include "Result.h"
#include "../trust_lines/TrustLine.h"

class PaymentResult : public Result {
private:
    trust_amount mAmount;
    uuids::uuid mContractorUUID;

public:
    PaymentResult(trust_amount amount,
                          boost::uuids::uuid contractorUUID,
                          Command *command,
                          uint16_t resultCode,
                          string timestampExcepted,
                          string timestampCompleted);

    ~PaymentResult();

    Command getCommand();

    uint16_t getResultCode();

    string getTimestampExcepted();

    string getTimestampCompleted();

    trust_amount getAmount();

    boost::uuids::uuid getContractorUUID();

    string serialize();
};

#endif //GEO_NETWORK_CLIENT_PAYMENTRESULT_H
