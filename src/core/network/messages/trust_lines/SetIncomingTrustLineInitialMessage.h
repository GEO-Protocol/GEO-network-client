#ifndef GEO_NETWORK_CLIENT_SETINCOMINGTRUSTLINEINITIALMESSAGE_H
#define GEO_NETWORK_CLIENT_SETINCOMINGTRUSTLINEINITIALMESSAGE_H

#include "SetIncomingTrustLineMessage.h"

class SetIncomingTrustLineInitialMessage : public SetIncomingTrustLineMessage {

public:
    typedef shared_ptr<SetIncomingTrustLineInitialMessage> Shared;

public:
    SetIncomingTrustLineInitialMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &sender,
        const TransactionUUID &transactionUUID,
        const NodeUUID &destinationMessage,
        const TrustLineAmount &amount,
        bool isContractorGateway)
    noexcept;

    SetIncomingTrustLineInitialMessage(
        BytesShared buffer)
        noexcept;

    const MessageType typeID() const
    noexcept;

    const bool isContractorGateway() const
    noexcept;

    virtual pair<BytesShared, size_t> serializeToBytes() const override;

protected:
    bool mIsContractorGateway;
};


#endif //GEO_NETWORK_CLIENT_SETINCOMINGTRUSTLINEINITIALMESSAGE_H
