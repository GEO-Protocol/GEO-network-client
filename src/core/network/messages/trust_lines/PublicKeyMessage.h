#ifndef GEO_NETWORK_CLIENT_PUBLICKEYMESSAGE_H
#define GEO_NETWORK_CLIENT_PUBLICKEYMESSAGE_H

#include "../base/transaction/TransactionMessage.h"
#include "../../../crypto/lamportkeys.h"

using namespace crypto;

class PublicKeyMessage : public TransactionMessage {

public:
    typedef shared_ptr<PublicKeyMessage> Shared;

public:
    PublicKeyMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const KeyNumber number,
        const lamport::PublicKey::Shared publicKey);

    PublicKeyMessage(
        BytesShared buffer);

    const KeyNumber number() const;

    const lamport::PublicKey::Shared publicKey() const;

    const MessageType typeID() const;

    const bool isAddToConfirmationRequiredMessagesHandler() const;

    const bool isCheckCachedResponse() const override;

    virtual pair<BytesShared, size_t> serializeToBytes() const;

private:
    KeyNumber mNumber;
    lamport::PublicKey::Shared mPublicKey;
};


#endif //GEO_NETWORK_CLIENT_PUBLICKEYMESSAGE_H
