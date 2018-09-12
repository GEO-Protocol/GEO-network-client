#ifndef GEO_NETWORK_CLIENT_SETTRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_SETTRUSTLINEMESSAGE_H

#include "../base/transaction/DestinationMessage.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"


class SetIncomingTrustLineMessage:
    public DestinationMessage {

public:
    typedef shared_ptr<SetIncomingTrustLineMessage> Shared;

public:
    SetIncomingTrustLineMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &sender,
        const TransactionUUID &transactionUUID,
        const NodeUUID &destinationMessage,
        const TrustLineAmount &amount)
        noexcept;

    SetIncomingTrustLineMessage(
        BytesShared buffer)
        noexcept;

    const MessageType typeID() const
        noexcept;

    const TrustLineAmount& amount() const
        noexcept;

    const bool isAddToConfirmationRequiredMessagesHandler() const override;

    const bool isCheckCachedResponse() const override;

    virtual pair<BytesShared, size_t> serializeToBytes() const;

protected:
    const size_t kOffsetToInheritedBytes() const
    noexcept;

protected:
    TrustLineAmount mAmount;
};


#endif //GEO_NETWORK_CLIENT_SETTRUSTLINEMESSAGE_H
