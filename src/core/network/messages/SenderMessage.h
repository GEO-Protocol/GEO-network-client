#ifndef GEO_NETWORK_CLIENT_SENDERMESSAGE_H
#define GEO_NETWORK_CLIENT_SENDERMESSAGE_H

#include "EquivalentMessage.h"

#include "../../common/multiprecision/MultiprecisionUtils.h"
#include "../../common/serialization/BytesDeserializer.h"
#include "../../common/serialization/BytesSerializer.h"

/*
 * Abstract base class for messages that must contain sender node address.
 */
class SenderMessage:
    public EquivalentMessage {

public:
    ContractorID idOnReceiverSide;
    vector<BaseAddress::Shared> senderAddresses;

public:
    SenderMessage(
        const SerializedEquivalent equivalent,
        ContractorID idOnReceiverSide,
        vector<BaseAddress::Shared> senderAddresses = {})
        noexcept;

    SenderMessage(
        const SerializedEquivalent equivalent,
        vector<BaseAddress::Shared> &senderAddresses)
        noexcept;

    SenderMessage(
        BytesShared buffer)
        noexcept;

    virtual pair<BytesShared, size_t> serializeToBytes() const override;

protected:
    virtual const size_t kOffsetToInheritedBytes() const override;
};

#endif //GEO_NETWORK_CLIENT_SENDERMESSAGE_H
