#ifndef GEO_NETWORK_CLIENT_PUBLICKEYHASHCONFIRMATION_H
#define GEO_NETWORK_CLIENT_PUBLICKEYHASHCONFIRMATION_H

#include "../base/transaction/ConfirmationMessage.h"
#include "../../../crypto/lamportkeys.h"

using namespace crypto;

class PublicKeyHashConfirmation : public ConfirmationMessage {

public:
    typedef shared_ptr<PublicKeyHashConfirmation> Shared;

public:
    PublicKeyHashConfirmation(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        KeyNumber number,
        lamport::KeyHash::Shared hashConfirmation);

    PublicKeyHashConfirmation(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        OperationState state);

    PublicKeyHashConfirmation(
        BytesShared buffer);

    const KeyNumber number() const;

    const lamport::KeyHash::Shared hashConfirmation() const;

    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes() const
        throw (bad_alloc);

private:
    KeyNumber mNumber;
    lamport::KeyHash::Shared mHashConfirmation;
};


#endif //GEO_NETWORK_CLIENT_PUBLICKEYHASHCONFIRMATION_H
