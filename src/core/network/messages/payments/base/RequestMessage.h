#ifndef REQUESTMESSAGE_H
#define REQUESTMESSAGE_H


#include "../../base/transaction/TransactionMessage.h"

#include "../../../../common/multiprecision/MultiprecisionUtils.h"


class RequestMessage:
    public TransactionMessage {

public:
    RequestMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const PathUUID &pathUUID,
        const TrustLineAmount &amount);

    RequestMessage(
        BytesShared buffer);

    const TrustLineAmount& amount() const;

    const PathUUID& pathUUID() const;

protected:
    virtual pair<BytesShared, size_t> serializeToBytes() const
        throw(bad_alloc);

    const size_t kOffsetToInheritedBytes() const
        noexcept;

    void deserializeFromBytes(
        BytesShared buffer);

protected:
    TrustLineAmount mAmount;
    PathUUID mPathUUID;
};

#endif // REQUESTMESSAGE_H
