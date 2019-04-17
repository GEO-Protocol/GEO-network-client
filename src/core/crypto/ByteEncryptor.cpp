#include "ByteEncryptor.h"

ByteEncryptor::ByteEncryptor(
    const ByteEncryptor::PublicKeyShared &publicKey) :
    mPublicKey(publicKey)
{}

ByteEncryptor::ByteEncryptor(
    const ByteEncryptor::PublicKeyShared &publicKey,
    const ByteEncryptor::SecretKeyShared &secretKey) :
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

static void parseHex(uint8_t *out, const string &in) {
    char b[3], i=0; b[2] = '\0';
    for(const char *p=in.c_str(),*e=p+in.length(); p<e; p+=2,++i) {
        memcpy(b, p, 2);
        out[(int)i] = std::stoul(b, nullptr, 16);
    }
}

std::string ByteEncryptor_parsePar(std::string &par, const std::string &separator) {
    if(par.find(separator) != std::string::npos) {
        std::string options = par.substr(par.find(separator)+1).c_str();
        par = par.substr(0, par.find(separator));
        return options;
    }
    return "";
}

ByteEncryptor::PublicKey::PublicKey(const string &str) {
    parseHex(key, str);
}

ByteEncryptor::SecretKey::SecretKey(const string &str) {
    parseHex(key, str);
}

ByteEncryptor::KeyPair::KeyPair(const string &str) {
    ;
}

std::ostream &operator<< (std::ostream &out, const ByteEncryptor::PublicKey &t) {
    std::stringstream ss;
    char buf[4];
    for(uint32_t i=0; i<crypto_box_PUBLICKEYBYTES; ++i) {
        sprintf(buf, "%02x", t.key[i]);
        ss << buf;
    }
    return (out << ss.str());
}
std::ostream &operator<< (std::ostream &out, const ByteEncryptor::SecretKey &t) {
    std::stringstream ss;
    char buf[4];
    for(uint32_t i=0; i<crypto_box_SECRETKEYBYTES; ++i) {
        sprintf(buf, "%02x", t.key[i]);
        ss << buf;
    }
    return (out << ss.str());
}
std::ostream &operator<< (std::ostream &out, const ByteEncryptor::KeyPair &t) {
    out << *t.publicKey
        << "_"
        << *t.secretKey;
    return out;
}
