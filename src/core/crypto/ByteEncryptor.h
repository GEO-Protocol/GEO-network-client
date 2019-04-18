#ifndef GEO_NETWORK_CLIENT_BYTEENCRYPTOR_H
#define GEO_NETWORK_CLIENT_BYTEENCRYPTOR_H

#include <sodium.h>
#include "../common/Types.h"
#include "../common/memory/MemoryUtils.h"

class ByteEncryptor {
public:
    typedef pair<BytesShared, size_t> Buffer;
    struct PublicKey {
        PublicKey() {}
        PublicKey(const string &str);
        uint8_t key[crypto_box_PUBLICKEYBYTES];
    };
    struct SecretKey {
        SecretKey() {}
        SecretKey(const string &str);
        uint8_t key[crypto_box_SECRETKEYBYTES];
    };
    typedef std::shared_ptr<PublicKey> PublicKeyShared;
    typedef std::shared_ptr<SecretKey> SecretKeyShared;

    struct KeyPair {
        KeyPair() {}
        KeyPair(const string &str);
        PublicKeyShared publicKey = NULL;
        SecretKeyShared secretKey = NULL;
    };

public:
    explicit ByteEncryptor(
        const PublicKeyShared &publicKey);
    ByteEncryptor(
        const PublicKeyShared &publicKey,
        const SecretKeyShared &secretKey);

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

std::ostream &operator<< (std::ostream &out, const ByteEncryptor::PublicKey &t);
std::ostream &operator<< (std::ostream &out, const ByteEncryptor::SecretKey &t);
std::ostream &operator<< (std::ostream &out, const ByteEncryptor::KeyPair &t);


#endif //GEO_NETWORK_CLIENT_BYTEENCRYPTOR_H
