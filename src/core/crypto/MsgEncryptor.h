//
// Created by minyor on 08.04.19.
//

#ifndef GEO_NETWORK_CLIENT_MSGENCRYPTOR_H
#define GEO_NETWORK_CLIENT_MSGENCRYPTOR_H


#include <sodium.h>
#include "../common/Types.h"

class MsgEncryptor {
public:
    typedef pair<BytesShared, size_t> Buffer;
    struct PublicKey { uint8_t key[crypto_box_PUBLICKEYBYTES+1]; };
    struct SecretKey { uint8_t key[crypto_box_SECRETKEYBYTES+1]; };
    typedef std::shared_ptr<PublicKey> PublicKeyShared;
    typedef std::shared_ptr<SecretKey> SecretKeyShared;

    struct KeyPair {
        PublicKeyShared publicKey = NULL;
        SecretKeyShared secretKey = NULL;
    };

public:
    explicit MsgEncryptor(
        PublicKeyShared publicKey);
    MsgEncryptor(
        PublicKeyShared publicKey,
        SecretKeyShared secretKey);

public:
    static KeyPair generateKeyPair();
    static KeyPair defaultKeyPair();

public:
    Buffer encrypt(const Buffer &msg) const;
    Buffer decrypt(const Buffer &cipher) const;

private:
    PublicKeyShared mPublicKey = NULL;
    SecretKeyShared mSecretKey = NULL;
};


#endif //GEO_NETWORK_CLIENT_MSGENCRYPTOR_H
