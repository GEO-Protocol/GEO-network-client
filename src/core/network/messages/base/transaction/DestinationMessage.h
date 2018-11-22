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
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const NodeUUID &destinationUUID)
    noexcept;

    DestinationMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        vector<BaseAddress::Shared> senderAddresses,
        const TransactionUUID &transactionUUID,
        const NodeUUID &destinationUUID)
    noexcept;

    DestinationMessage(
        BytesShared buffer)
    noexcept;

    pair<BytesShared, size_t> serializeToBytes() const override;

    const NodeUUID &destinationUUID() const
    noexcept;

    const bool isDestinationMessage() const override;

protected:
    const size_t kOffsetToInheritedBytes() const
    noexcept;

protected:
    NodeUUID mDestinationUUID;

};


#endif //GEO_NETWORK_CLIENT_DESTINATIONMESSAGE_H
