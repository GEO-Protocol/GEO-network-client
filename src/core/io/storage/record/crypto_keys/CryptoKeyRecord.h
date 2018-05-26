#ifndef GEO_NETWORK_CLIENT_CRYPTOKEYRECORD_H
#define GEO_NETWORK_CLIENT_CRYPTOKEYRECORD_H

#include "../../../../common/Types.h"

class CryptoKeyRecord {

public:
    typedef shared_ptr<CryptoKeyRecord> Shared;

public:
    CryptoKeyRecord(
        BytesShared publicKey,
        size_t publicKeySize,
        BytesShared privateKey,
        size_t privateKeySize,
        uint32_t number);

    const pair<BytesShared, size_t> publicKey() const;

    uint32_t publicKeyHash() const;

    const pair<BytesShared, size_t> privateKey() const;

    const uint32_t number() const;

private:
    BytesShared mPublicKey;
    size_t mPublicKeySize;
    BytesShared mPrivateKey;
    size_t mPrivateKeySize;
    uint32_t mNumber;
};


#endif //GEO_NETWORK_CLIENT_CRYPTOKEYRECORD_H
