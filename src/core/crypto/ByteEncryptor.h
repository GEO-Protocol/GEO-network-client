#ifndef GEO_NETWORK_CLIENT_BYTEENCRYPTOR_H
#define GEO_NETWORK_CLIENT_BYTEENCRYPTOR_H

#include <sodium.h>
#include "../common/Types.h"
#include "../common/memory/MemoryUtils.h"

class ByteEncryptor {
public:
    typedef pair<BytesShared, size_t> Buffer;
    struct PublicKey { uint8_t key[crypto_box_PUBLICKEYBYTES]; };
    struct SecretKey { uint8_t key[crypto_box_SECRETKEYBYTES]; };
    typedef std::shared_ptr<PublicKey> PublicKeyShared;
    typedef std::shared_ptr<SecretKey> SecretKeyShared;

    struct KeyPair {
        PublicKeyShared publicKey = NULL;
        SecretKeyShared secretKey = NULL;
    };

public:
    explicit ByteEncryptor(
        PublicKeyShared publicKey);
    ByteEncryptor(
        PublicKeyShared publicKey,
    SecretKeyShared secretKey);

public:
    static KeyPair generateKeyPair();
    static KeyPair defaultKeyPair();

public:
    Buffer encrypt(byte *bytes, size_t size, size_t headerSize = 0) const;
    Buffer decrypt(byte *cipher, size_t size, size_t headerSize = 0) const;
    Buffer encrypt(const Buffer &bytes) const;
    Buffer decrypt(const Buffer &cipher) const;

protected:
    PublicKeyShared mPublicKey = NULL;
    SecretKeyShared mSecretKey = NULL;
};


#endif //GEO_NETWORK_CLIENT_BYTEENCRYPTOR_H
