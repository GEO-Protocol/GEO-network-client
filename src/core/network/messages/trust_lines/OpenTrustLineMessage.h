#ifndef GEO_NETWORK_CLIENT_OPENTRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_OPENTRUSTLINEMESSAGE_H

#include "../base/transaction/TransactionMessage.h"

#include "../../../common/multiprecision/MultiprecisionUtils.h"


class OpenTrustLineMessage:
    public TransactionMessage {

public:
    OpenTrustLineMessage(
        const NodeUUID &sender,
        const TransactionUUID &transactionUUID,
        const TrustLineAmount &amount)
        noexcept;

protected:
    const MessageType typeID() const
        noexcept;

    pair<BytesShared, size_t> serializeToBytes() const
        throw (bad_alloc);

protected:
    const TrustLineAmount mTrustLineAmount;
};


#endif //GEO_NETWORK_CLIENT_OPENTRUSTLINEMESSAGE_H
