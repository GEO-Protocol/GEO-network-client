#ifndef GEO_NETWORK_CLIENT_DESTINATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_DESTINATIONMESSAGE_H

#include "TransactionMessage.h"

class DestinationMessage : public TransactionMessage {

public:
    typedef shared_ptr<DestinationMessage> Shared;
    typedef shared_ptr<const DestinationMessage> ConstShared;

public:
    DestinationMessage(
        const SerializedEquivalent equivalent,
        ContractorID idOnSenderSide,
        vector<BaseAddress::Shared> senderAddresses,
        const TransactionUUID &transactionUUID,
        ContractorID destinationID)
    noexcept;

    DestinationMessage(
        const SerializedEquivalent equivalent,
        ContractorID idOnSenderSide,
        const TransactionUUID &transactionUUID,
        ContractorID destinationID)
    noexcept;

    DestinationMessage(
        BytesShared buffer)
    noexcept;

    pair<BytesShared, size_t> serializeToBytes() const override;

    const ContractorID destinationID() const
    noexcept;

    const bool isDestinationMessage() const override;

protected:
    const size_t kOffsetToInheritedBytes() const
    noexcept;

protected:
    ContractorID mDestinationID;
};


#endif //GEO_NETWORK_CLIENT_DESTINATIONMESSAGE_H
