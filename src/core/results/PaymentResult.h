#ifndef GEO_NETWORK_CLIENT_PAYMENTRESULT_H
#define GEO_NETWORK_CLIENT_PAYMENTRESULT_H

#include "Result.h"

class PaymentResult : public Result {
private:
    uuids::uuid mTransactionUUID;
    NodeUUID mContractorUUID;
    trust_amount mAmount;
    string mPurpose;

public:
    PaymentResult(BaseUserCommand *command,
                  const uint16_t &resultCode,
                  const string &timestampCompleted,
                  const uuids::uuid &transactionUUID,
                  const NodeUUID &contractorUUID,
                  const trust_amount &amount,
                  const string &purpose);

    const uuids::uuid &commandUUID() const;

    const string &id() const;

    const uint16_t &resultCode() const;

    const string &timestampExcepted() const;

    const string &timestampCompleted() const;

    const uuids::uuid &transactionUUID() const;

    const NodeUUID &contractorUUID() const;

    const trust_amount &amount() const;

    const string &purpose() const;

    string serialize();
};

#endif //GEO_NETWORK_CLIENT_PAYMENTRESULT_H
