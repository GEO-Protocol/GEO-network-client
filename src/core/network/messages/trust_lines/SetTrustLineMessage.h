#ifndef GEO_NETWORK_CLIENT_SETTRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_SETTRUSTLINEMESSAGE_H

#include "../base/transaction/TransactionMessage.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"


class SetTrustLineMessage:
    public TransactionMessage {

public:
    SetTrustLineMessage(
        const NodeUUID &sender,
        const TransactionUUID &transactionUUID,
        const TrustLineAmount &newAmount)
        noexcept;

    const MessageType typeID() const
        noexcept;

    virtual pair<BytesShared, size_t> serializeToBytes() const
        throw (bad_alloc);

private:
    const TrustLineAmount mNewTrustLineAmount;
};


#endif //GEO_NETWORK_CLIENT_SETTRUSTLINEMESSAGE_H
