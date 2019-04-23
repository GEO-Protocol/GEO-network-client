#include "ByteEncryptor.h"

ByteEncryptor::ByteEncryptor(
    const ByteEncryptor::PublicKey::Shared &publicKey) :
    mPublicKey(publicKey)
{}

ByteEncryptor::ByteEncryptor(
    const ByteEncryptor::PublicKey::Shared &publicKey,
    const ByteEncryptor::SecretKey::Shared &secretKey) :
    mPublicKey(publicKey),
    mSecretKey(secretKey)
{}

ByteEncryptor::KeyPair::Shared ByteEncryptor::generateKeyPair()
{
    KeyPair::Shared keyPair = make_shared<KeyPair>();
    keyPair->publicKey = std::make_shared<PublicKey>();
    keyPair->secretKey = std::make_shared<SecretKey>();
    crypto_box_keypair(
        keyPair->publicKey->key,
        keyPair->secretKey->key);
    return keyPair;
}

ByteEncryptor::Buffer ByteEncryptor::encrypt(
    byte *bytes,
    size_t size,
    size_t headerSize) const
{
    if(!mPublicKey) {
        return ByteEncryptor::Buffer(nullptr, 0);
    }
    size_t len = size + crypto_box_SEALBYTES + headerSize;
    ByteEncryptor::Buffer cipher(
        tryMalloc(len),
        len);
    crypto_box_seal(
        cipher.first.get() + headerSize,
        bytes,
        size,
        mPublicKey->key);
    return cipher;
}

ByteEncryptor::Buffer ByteEncryptor::decrypt(
    byte *cipher,
    size_t size,
    size_t headerSize) const
{
    if(!mPublicKey || !mSecretKey) {
        return ByteEncryptor::Buffer(nullptr, 0);
    }
    size_t len = (size - crypto_box_SEALBYTES) + headerSize;
    ByteEncryptor::Buffer bytes(
        tryMalloc(len),
        len);
    auto result = crypto_box_seal_open(
        bytes.first.get() + headerSize,
        cipher,
        size,
        mPublicKey->key,
        mSecretKey->key);
    return result ?
        ByteEncryptor::Buffer(nullptr, 0) :
        bytes;
}

ByteEncryptor::Buffer ByteEncryptor::encrypt(
    const ByteEncryptor::Buffer &bytes) const
{
    return encrypt(
        bytes.first.get(),
        bytes.second);
}

ByteEncryptor::Buffer ByteEncryptor::decrypt(
    const ByteEncryptor::Buffer &cipher) const
{
    return decrypt(
        cipher.first.get(),
        cipher.second);
}

static void parseHex(
    byte *out,
    const string &in)
{
    char b[3], i=0; b[2] = '\0';
    for(const char *p=in.c_str(),*e=p+in.length(); p<e; p+=2,++i) {
        memcpy(b, p, 2);
        out[i] = (byte)std::stoul(b, nullptr, 16);
    }
}

std::string ByteEncryptor_parsePar(
    std::string &par,
    const std::string &separator)
{
    if(par.find(separator) != std::string::npos) {
        std::string options = par.substr(par.find(separator)+1);
        par = par.substr(0, par.find(separator));
        return options;
    }
    return "";
}

ByteEncryptor::PublicKey::PublicKey(
    const string &str)
{
    parseHex(key, str);
}

ByteEncryptor::SecretKey::SecretKey(
    const string &str)
{
    parseHex(key, str);
}

ByteEncryptor::KeyPair::KeyPair(const string &str) {
    ;
}

std::ostream &operator<< (
    std::ostream &out,
    const ByteEncryptor::PublicKey &t)
{
    std::stringstream ss;
    char buf[4];
    for(byte i: t.key) {
        sprintf(buf, "%02x", i);
        ss << buf;
    }
    return (out << ss.str());
}

std::ostream &operator<< (
    std::ostream &out,
    const ByteEncryptor::SecretKey &t)
{
    std::stringstream ss;
    char buf[4];
    for(byte i: t.key) {
        sprintf(buf, "%02x", i);
        ss << buf;
    }
    return (out << ss.str());
}

std::ostream &operator<< (
    std::ostream &out,
    const ByteEncryptor::KeyPair &t)
{
    out << *t.publicKey
        << "_"
        << *t.secretKey;
    return out;
}
