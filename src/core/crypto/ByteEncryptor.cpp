//
// Created by minyor on 08.04.19.
//

#include "ByteEncryptor.h"

ByteEncryptor::ByteEncryptor(
    ByteEncryptor::PublicKeyShared publicKey) :
    mPublicKey(publicKey)
{}

ByteEncryptor::ByteEncryptor(
    ByteEncryptor::PublicKeyShared publicKey,
    ByteEncryptor::SecretKeyShared secretKey) :
    mPublicKey(publicKey),
    mSecretKey(secretKey)
{}

ByteEncryptor::KeyPair ByteEncryptor::generateKeyPair() {
    ByteEncryptor::KeyPair keyPair;
    keyPair.publicKey = std::make_shared<PublicKey>();
    keyPair.secretKey = std::make_shared<SecretKey>();
    crypto_box_keypair(
        keyPair.publicKey->key,
        keyPair.secretKey->key);
    keyPair.publicKey->key[crypto_box_PUBLICKEYBYTES] = 0;
    keyPair.secretKey->key[crypto_box_SECRETKEYBYTES] = 0;
    return keyPair;
}

ByteEncryptor::KeyPair ByteEncryptor::defaultKeyPair() {
    ByteEncryptor::KeyPair keyPair;
    keyPair.publicKey = std::make_shared<PublicKey>();
    keyPair.secretKey = std::make_shared<SecretKey>();
    uint8_t p[] = { 0xc1, 0x0f, 0xab, 0xe1, 0x1e, 0x13, 0x87, 0x42, 0x77, 0x30, 0x69, 0xaf, 0x68, 0x83, 0x40, 0xd6, 0x63, 0xd9, 0x2e, 0x51, 0xe7, 0xe1, 0xca, 0x57, 0x26, 0x9f, 0x66, 0xde, 0x33, 0x82, 0x8d, 0x2a };
    uint8_t s[] = { 0xe2, 0xa3, 0xce, 0x5f, 0x2d, 0x51, 0xa1, 0xc8, 0x4b, 0xe8, 0x40, 0x8e, 0xa3, 0xa6, 0x0d, 0xca, 0xb2, 0xd0, 0xa4, 0xcc, 0xdb, 0xe7, 0xf4, 0x21, 0x32, 0x3d, 0x72, 0x5f, 0x01, 0x12, 0xc2, 0x0f };
    memcpy(keyPair.publicKey->key, p, sizeof(p));
    memcpy(keyPair.secretKey->key, s, sizeof(s));
    return keyPair;
}

ByteEncryptor::Buffer ByteEncryptor::encrypt(byte *bytes, size_t size, size_t headerSize) const {
    if(!mPublicKey) ByteEncryptor::Buffer(NULL, 0);
    size_t len = size + crypto_box_SEALBYTES + headerSize;
    ByteEncryptor::Buffer cipher(
        tryMalloc(len),
        len
    );
    crypto_box_seal(
        cipher.first.get() + headerSize,
        bytes,
        size,
        mPublicKey->key);
    return cipher;
}

ByteEncryptor::Buffer ByteEncryptor::decrypt(byte *cipher, size_t size, size_t headerSize) const {
    if(!mPublicKey || !mSecretKey) ByteEncryptor::Buffer(NULL, 0);
    size_t len = (size - crypto_box_SEALBYTES) + headerSize;
    ByteEncryptor::Buffer bytes(
        tryMalloc(len),
        len
    );
    auto result = crypto_box_seal_open(
        bytes.first.get() + headerSize,
        cipher,
        size,
        mPublicKey->key,
        mSecretKey->key);
    return result ?
        ByteEncryptor::Buffer(NULL, 0) :
        bytes;
}

ByteEncryptor::Buffer ByteEncryptor::encrypt(const ByteEncryptor::Buffer &bytes) const {
    return encrypt(bytes.first.get(), bytes.second);
}

ByteEncryptor::Buffer ByteEncryptor::decrypt(const ByteEncryptor::Buffer &cipher) const {
    return decrypt(cipher.first.get(), cipher.second);
}
