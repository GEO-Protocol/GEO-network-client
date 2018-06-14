#ifndef GEO_NETWORK_CLIENT_PUBLICKEYCRCCONFIRMATION_H
#define GEO_NETWORK_CLIENT_PUBLICKEYCRCCONFIRMATION_H

#include "../base/transaction/TransactionMessage.h"

class PublicKeyCRCConfirmation : public TransactionMessage {

public:
    typedef shared_ptr<PublicKeyCRCConfirmation> Shared;

public:
    PublicKeyCRCConfirmation(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        KeyNumber number,
        uint64_t crcConfirmation);

    PublicKeyCRCConfirmation(
        BytesShared buffer);

    const KeyNumber number() const;

    const uint64_t crcConfirmation() const;

    const MessageType typeID() const
        noexcept;

    virtual pair<BytesShared, size_t> serializeToBytes() const;

private:
    KeyNumber mNumber;
    uint64_t mCrcConfirmation;
};


#endif //GEO_NETWORK_CLIENT_PUBLICKEYCRCCONFIRMATION_H
