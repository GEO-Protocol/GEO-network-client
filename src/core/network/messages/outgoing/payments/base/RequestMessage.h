#ifndef REQUESTMESSAGE_H
#define REQUESTMESSAGE_H


#include "../../../base/transaction/TransactionMessage.h"

#include "../../../../../common/multiprecision/MultiprecisionUtils.h"


class RequestMessage:
    public TransactionMessage {

public:
    RequestMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const TrustLineAmount &amount);

    RequestMessage(
        BytesShared buffer);

    const TrustLineAmount& amount() const;

protected:
    pair<BytesShared, size_t> serializeToBytes();

    static const size_t kOffsetToInheritedBytes();

    void deserializeFromBytes(
        BytesShared buffer);

protected:
    TrustLineAmount mAmount;
};

#endif // REQUESTMESSAGE_H
