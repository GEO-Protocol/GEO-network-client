#include "MsgEncryptor.h"

MsgEncryptor::KeyTrio::Shared MsgEncryptor::generateKeyTrio(
    const string &outputKey)
{
    KeyTrio::Shared keyTrio = make_shared<KeyTrio>();
    auto keyPair = generateKeyPair();
    keyTrio->publicKey = keyPair->publicKey;
    keyTrio->secretKey = keyPair->secretKey;

    if(!outputKey.empty()) {
        keyTrio->contractorPublicKey = std::make_shared<PublicKey>(outputKey);
    }
    return keyTrio;
}

ByteEncryptor::Buffer MsgEncryptor::encrypt(
    Message::Shared message)
{
    auto bytesAndBytesCount = message->serializeToBytes();
    auto pair = ByteEncryptor::encrypt(
        bytesAndBytesCount.first.get() + Message::UnencryptedHeaderSize,
        bytesAndBytesCount.second - Message::UnencryptedHeaderSize,
        Message::UnencryptedHeaderSize);
    memcpy(
        pair.first.get(),
        bytesAndBytesCount.first.get(),
        Message::UnencryptedHeaderSize);
    return pair;
}

ByteEncryptor::Buffer MsgEncryptor::decrypt(
    BytesShared buffer,
    const size_t count)
{
    auto pair = ByteEncryptor::decrypt(
        buffer.get() + Message::UnencryptedHeaderSize,
        count - Message::UnencryptedHeaderSize,
        Message::UnencryptedHeaderSize);
    memcpy(
        pair.first.get(),
        buffer.get(),
        Message::UnencryptedHeaderSize);
    return pair;
}

void MsgEncryptor::KeyTrio::serialize(
    vector<byte> &out) const
{
    out.resize(PublicKey::kBytesSize * 2 + SecretKey::kBytesSize);
    byte *p = &out[0];
    mempcpy(
        p,
        publicKey->key,
        PublicKey::kBytesSize);
    p += PublicKey::kBytesSize;
    mempcpy(
        p,
        secretKey->key,
        SecretKey::kBytesSize);
    p += SecretKey::kBytesSize;
    if(contractorPublicKey) {
        mempcpy(
            p,
            contractorPublicKey->key,
            PublicKey::kBytesSize);
    }
}

std::string ByteEncryptor_parsePar(
    std::string &par,
    const std::string &separator);

MsgEncryptor::KeyTrio::KeyTrio()
{}

MsgEncryptor::KeyTrio::KeyTrio(
    const string &str)
{
    std::string p1 = str;
    std::string p2 = ByteEncryptor_parsePar(p1, "_");
    if(!p2.empty()) {
        std::string p3 = ByteEncryptor_parsePar(p2, "_");
        if(!p3.empty()) {
            contractorPublicKey = std::make_shared<PublicKey>(p3);
        }
        secretKey = std::make_shared<SecretKey>(p2);
    }
    publicKey = std::make_shared<PublicKey>(p1);
}

void MsgEncryptor::KeyTrio::deserialize(
    const vector<byte> &in)
{
    uint32_t pairSize = PublicKey::kBytesSize + SecretKey::kBytesSize;
    const byte *p = &in[0];
    if(in.size() >= PublicKey::kBytesSize) {
        publicKey = std::make_shared<PublicKey>();
        mempcpy(
            publicKey->key,
            p,
            PublicKey::kBytesSize);
        p += PublicKey::kBytesSize;
    }
    if(in.size() >= pairSize) {
        secretKey = std::make_shared<SecretKey>();
        mempcpy(
            secretKey->key,
            p,
            SecretKey::kBytesSize);
        p += SecretKey::kBytesSize;
    }
    if(in.size() > (pairSize + PublicKey::kBytesSize)) {
        contractorPublicKey = std::make_shared<PublicKey>();
        mempcpy(
            contractorPublicKey->key,
            p,
            PublicKey::kBytesSize);
    }
}

std::ostream &operator<< (
    std::ostream &out,
    const MsgEncryptor::KeyTrio &t)
{
    out << (const MsgEncryptor::KeyPair &)t;
    if(!t.contractorPublicKey) return out;
    out << "_"
        << *t.contractorPublicKey;
    return out;
}
