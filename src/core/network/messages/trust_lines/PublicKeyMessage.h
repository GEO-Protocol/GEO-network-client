#ifndef GEO_NETWORK_CLIENT_PUBLICKEYMESSAGE_H
#define GEO_NETWORK_CLIENT_PUBLICKEYMESSAGE_H

#include "../base/transaction/TransactionMessage.h"
#include "../../../crypto/CryptoKey.h"

class PublicKeyMessage : public TransactionMessage {

public:
    typedef shared_ptr<PublicKeyMessage> Shared;

public:
    PublicKeyMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        uint32_t number,
        const CryptoKey &publicKey);

    PublicKeyMessage(
        BytesShared buffer);

    const uint32_t number() const;

    const CryptoKey& publicKey() const;

    const MessageType typeID() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const;

private:
    uint32_t mNumber;
    CryptoKey mPublicKey;
};


#endif //GEO_NETWORK_CLIENT_PUBLICKEYMESSAGE_H
