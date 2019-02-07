#ifndef GEO_NETWORK_CLIENT_REQUESTCYCLEMESSAGE_H
#define GEO_NETWORK_CLIENT_REQUESTCYCLEMESSAGE_H

#include "../../base/transaction/TransactionMessage.h"

#include "../../../../common/multiprecision/MultiprecisionUtils.h"

class RequestCycleMessage : public TransactionMessage {

public:
    RequestCycleMessage(
        const SerializedEquivalent equivalent,
        vector<BaseAddress::Shared> &senderAddresses,
        const TransactionUUID &transactionUUID,
        const TrustLineAmount &amount);

    RequestCycleMessage(
        BytesShared buffer);

    const TrustLineAmount& amount() const;

protected:
    virtual pair<BytesShared, size_t> serializeToBytes() const override;

    const size_t kOffsetToInheritedBytes() const override;

protected:
    TrustLineAmount mAmount;
};


#endif //GEO_NETWORK_CLIENT_REQUESTCYCLEMESSAGE_H
