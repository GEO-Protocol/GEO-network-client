#ifndef GEO_NETWORK_CLIENT_SETTRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_SETTRUSTLINEMESSAGE_H

#include "../base/transaction/TransactionMessage.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"


class SetIncomingTrustLineMessage:
    public TransactionMessage {

public:
    typedef shared_ptr<SetIncomingTrustLineMessage> Shared;

public:
    SetIncomingTrustLineMessage(
        const NodeUUID &sender,
        const TransactionUUID &transactionUUID,
        const TrustLineAmount &amount)
        noexcept;

    SetIncomingTrustLineMessage(
        BytesShared buffer)
        noexcept;

    const MessageType typeID() const
        noexcept;

    const TrustLineAmount& amount() const
        noexcept;

    virtual pair<BytesShared, size_t> serializeToBytes() const;

protected:
    TrustLineAmount mAmount;
};


#endif //GEO_NETWORK_CLIENT_SETTRUSTLINEMESSAGE_H
