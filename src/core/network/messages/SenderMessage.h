#ifndef GEO_NETWORK_CLIENT_SENDERMESSAGE_H
#define GEO_NETWORK_CLIENT_SENDERMESSAGE_H

#include "EquivalentMessage.h"

#include "../../common/NodeUUID.h"
#include "../../contractors/addresses/IPv4WithPortAddress.h"
#include "../../common/serialization/BytesDeserializer.h"
#include "../../common/serialization/BytesSerializer.h"

/*
 * Abstract base class for messages that must contain sender node UUID.
 */
class SenderMessage:
    public EquivalentMessage {

public:
    const NodeUUID senderUUID;
    ContractorID idOnSenderSide;
    vector<BaseAddress::Shared> senderAddresses;

public:
    SenderMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        ContractorID idOnSenderSide,
        vector<BaseAddress::Shared> senderAddresses = {})
        noexcept;

    SenderMessage(
        BytesShared buffer)
        noexcept;

    virtual pair<BytesShared, size_t> serializeToBytes() const;

protected:
    virtual const size_t kOffsetToInheritedBytes() const
        noexcept;
};

#endif //GEO_NETWORK_CLIENT_SENDERMESSAGE_H
