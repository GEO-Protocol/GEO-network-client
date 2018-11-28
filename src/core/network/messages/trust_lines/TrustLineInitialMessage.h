#ifndef GEO_NETWORK_CLIENT_TRUSTLINEINITIALMESSAGE_H
#define GEO_NETWORK_CLIENT_TRUSTLINEINITIALMESSAGE_H

#include "../base/transaction/DestinationMessage.h"

class TrustLineInitialMessage : public DestinationMessage {

public:
    typedef shared_ptr<TrustLineInitialMessage> Shared;

public:
    TrustLineInitialMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &sender,
        ContractorID idOnSenderSide,
        vector<BaseAddress::Shared> senderAddresses,
        const TransactionUUID &transactionUUID,
        const NodeUUID &destinationUUID,
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
