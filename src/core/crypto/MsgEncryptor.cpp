//
// Created by minyor on 08.04.19.
//

#include "MsgEncryptor.h"

MsgEncryptor::MsgEncryptor(
    MsgEncryptor::PublicKeyShared publicKey) :
    mPublicKey(publicKey) {}

MsgEncryptor::MsgEncryptor(
    MsgEncryptor::PublicKeyShared publicKey,
    MsgEncryptor::SecretKeyShared secretKey) :
    mPublicKey(publicKey),
    mSecretKey(secretKey)
{}

MsgEncryptor::KeyPair MsgEncryptor::generateKeyPair() {
    MsgEncryptor::KeyPair keyPair;
    keyPair.publicKey = std::make_shared<PublicKey>();
    keyPair.secretKey = std::make_shared<SecretKey>();
    crypto_box_keypair(
        keyPair.publicKey->key,
        keyPair.secretKey->key);
    keyPair.publicKey->key[crypto_box_PUBLICKEYBYTES] = 0;
    keyPair.secretKey->key[crypto_box_SECRETKEYBYTES] = 0;
    return keyPair;
}

MsgEncryptor::KeyPair MsgEncryptor::defaultKeyPair() {
    MsgEncryptor::KeyPair keyPair;
    keyPair.publicKey = std::make_shared<PublicKey>();
    keyPair.secretKey = std::make_shared<SecretKey>();
    strcpy((char *)keyPair.publicKey->key, "rthujdfk54y5y503909ikhoilf409yk6");
    strcpy((char *)keyPair.secretKey->key, "sftg8534k89jtf92l67u7189m8n6cf");
    return keyPair;
}

MsgEncryptor::Buffer MsgEncryptor::encrypt(const MsgEncryptor::Buffer &msg) const {
    if(!mPublicKey) MsgEncryptor::Buffer(NULL, 0);
    size_t len = msg.second + crypto_box_SEALBYTES;
    MsgEncryptor::Buffer cipher(
        std::shared_ptr<byte>(new byte[len]),
        len
    );
    crypto_box_seal(
        cipher.first.get(),
        msg.first.get(),
        msg.second,
        mPublicKey->key);
    return cipher;
}

MsgEncryptor::Buffer MsgEncryptor::decrypt(const MsgEncryptor::Buffer &cipher) const {
    if(!mPublicKey || !mSecretKey) MsgEncryptor::Buffer(NULL, 0);
    size_t len = cipher.second - crypto_box_SEALBYTES;
    MsgEncryptor::Buffer msg(
        std::shared_ptr<byte>(new byte[len]),
        len
    );
    auto result = crypto_box_seal_open(
        msg.first.get(),
        cipher.first.get(),
        cipher.second,
        mPublicKey->key,
        mSecretKey->key);
    return result ?
        MsgEncryptor::Buffer(NULL, 0) :
        msg;
}
