#ifndef REQUESTMESSAGE_H
#define REQUESTMESSAGE_H


#include "../../base/transaction/TransactionMessage.h"

#include "../../../../common/multiprecision/MultiprecisionUtils.h"


class RequestMessage:
    public TransactionMessage {

public:
    RequestMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const PathID &pathID,
        const TrustLineAmount &amount);

    RequestMessage(
        BytesShared buffer);

    const TrustLineAmount& amount() const;

    const PathID& pathID() const;

protected:
    virtual pair<BytesShared, size_t> serializeToBytes() const
        throw(bad_alloc);

    const size_t kOffsetToInheritedBytes() const
        noexcept;

protected:
    TrustLineAmount mAmount;
    PathID mPathID;
};

#endif // REQUESTMESSAGE_H
