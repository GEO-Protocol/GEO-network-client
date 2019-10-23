#ifndef GEO_NETWORK_CLIENT_TRUSTLINEINITIALMESSAGE_H
#define GEO_NETWORK_CLIENT_TRUSTLINEINITIALMESSAGE_H

#include "../base/transaction/TransactionMessage.h"

class TrustLineInitialMessage : public TransactionMessage {

public:
    typedef shared_ptr<TrustLineInitialMessage> Shared;

public:
    TrustLineInitialMessage(
        const SerializedEquivalent equivalent,
        Contractor::Shared contractor,
        const TransactionUUID &transactionUUID,
        bool isContractorGateway);

    TrustLineInitialMessage(
        BytesShared buffer);

    const MessageType typeID() const override;

    const bool isContractorGateway() const;

    const bool isCheckCachedResponse() const override;

    pair<BytesShared, size_t> serializeToBytes() const override;

protected:
    bool mIsContractorGateway;
};


#endif //GEO_NETWORK_CLIENT_TRUSTLINEINITIALMESSAGE_H
