#ifndef GEO_NETWORK_CLIENT_REQUESTCYCLEMESSAGE_H
#define GEO_NETWORK_CLIENT_REQUESTCYCLEMESSAGE_H

#include "../../base/transaction/TransactionMessage.h"

#include "../../../../common/multiprecision/MultiprecisionUtils.h"

class RequestCycleMessage : public TransactionMessage {

public:
    RequestCycleMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const TrustLineAmount &amount);

    RequestCycleMessage(
        BytesShared buffer);

    const TrustLineAmount& amount() const;

protected:
    virtual pair<BytesShared, size_t> serializeToBytes() const
    throw(bad_alloc);

    const size_t kOffsetToInheritedBytes() const
    noexcept;

protected:
    TrustLineAmount mAmount;
};


#endif //GEO_NETWORK_CLIENT_REQUESTCYCLEMESSAGE_H
