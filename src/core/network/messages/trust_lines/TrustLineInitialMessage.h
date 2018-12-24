#ifndef GEO_NETWORK_CLIENT_TRUSTLINEINITIALMESSAGE_H
#define GEO_NETWORK_CLIENT_TRUSTLINEINITIALMESSAGE_H

#include "../base/transaction/TransactionMessage.h"

class TrustLineInitialMessage : public TransactionMessage {

public:
    typedef shared_ptr<TrustLineInitialMessage> Shared;

public:
    TrustLineInitialMessage(
        const SerializedEquivalent equivalent,
        ContractorID idOnSenderSide,
        vector<BaseAddress::Shared> senderAddresses,
        const TransactionUUID &transactionUUID,
        ContractorID contractorID,
        bool isContractorGateway)
    noexcept;

    TrustLineInitialMessage(
        BytesShared buffer)
        noexcept;

    const MessageType typeID() const
    noexcept;

    const ContractorID contractorID() const
    noexcept;

    const bool isContractorGateway() const
    noexcept;

    const bool isCheckCachedResponse() const override;

    pair<BytesShared, size_t> serializeToBytes() const override;

protected:
    ContractorID mContractorID;
    bool mIsContractorGateway;
};


#endif //GEO_NETWORK_CLIENT_TRUSTLINEINITIALMESSAGE_H
