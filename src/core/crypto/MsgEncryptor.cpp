#include "MsgEncryptor.h"

MsgEncryptor::MsgEncryptor() :
    ByteEncryptor(
        defaultKeyPair().publicKey,
        defaultKeyPair().secretKey)
{}

MsgEncryptor::KeyTrio MsgEncryptor::generateKeyTrio(const string &outputKey) {
    MsgEncryptor::KeyTrio keyTrio;
    (ByteEncryptor::KeyPair &)keyTrio = generateKeyPair();
    if(!outputKey.empty())
        keyTrio.outputKey = std::make_shared<PublicKey>(outputKey);
    return keyTrio;
}

ByteEncryptor::Buffer MsgEncryptor::encrypt(Message::Shared message) {
    auto bytesAndBytesCount = message->serializeToBytes();

    auto pair = ByteEncryptor::encrypt(
        bytesAndBytesCount.first.get() + Message::UnencryptedHeaderSize,
        bytesAndBytesCount.second - Message::UnencryptedHeaderSize,
        Message::UnencryptedHeaderSize
    );
    memcpy(
        pair.first.get(),
        bytesAndBytesCount.first.get(),
        Message::UnencryptedHeaderSize);
    return pair;
}

ByteEncryptor::Buffer MsgEncryptor::decrypt(BytesShared buffer, const size_t count) {
    auto pair = ByteEncryptor::decrypt(
        buffer.get() + Message::UnencryptedHeaderSize,
        count - Message::UnencryptedHeaderSize,
        Message::UnencryptedHeaderSize
    );
    memcpy(
        pair.first.get(),
        buffer.get(),
        Message::UnencryptedHeaderSize);
    return pair;
}

void MsgEncryptor::KeyTrio::serialize(vector<uint8_t> &out) const {
    out.resize(crypto_box_PUBLICKEYBYTES * 2 + crypto_box_SECRETKEYBYTES);
    uint8_t *p = &out[0];
    mempcpy(
        p,
        publicKey->key,
        crypto_box_PUBLICKEYBYTES
    );
    p += crypto_box_PUBLICKEYBYTES;
    mempcpy(
        p,
        secretKey->key,
        crypto_box_SECRETKEYBYTES
    );
    p += crypto_box_SECRETKEYBYTES;
    if(outputKey) {
        mempcpy(
            p,
            outputKey->key,
            crypto_box_PUBLICKEYBYTES
        );
    }
}

std::string ByteEncryptor_parsePar(std::string &par, const std::string &separator);

MsgEncryptor::KeyTrio::KeyTrio() {
    static bool test = true;
    if(test) {
        test = false;
        auto keys = MsgEncryptor::generateKeyTrio();
        stringstream ss;
        ss << keys;
        fprintf(stdout, "TEST trio1: %s\n", ss.str().c_str());
        stringstream ss2;
        ss2 << KeyTrio(ss.str());
        fprintf(stdout, "TEST trio2: %s\n", ss2.str().c_str());
        stringstream ss3;
        ss3 << KeyTrio(ss2.str());
        fprintf(stdout, "TEST trio3: %s\n", ss3.str().c_str());
    }
}

MsgEncryptor::KeyTrio::KeyTrio(const string &str) {
    std::string p1 = str;
    std::string p2 = ByteEncryptor_parsePar(p1, "_");
    if(!p2.empty()) {
        std::string p3 = ByteEncryptor_parsePar(p2, "_");
        if(!p3.empty()) {
            outputKey = std::make_shared<PublicKey>(p3);
        }
        secretKey = std::make_shared<SecretKey>(p2);
    }
    publicKey = std::make_shared<PublicKey>(p1);
}

void MsgEncryptor::KeyTrio::deserialize(const vector<uint8_t> &in) {
    uint32_t pairSize = crypto_box_PUBLICKEYBYTES + crypto_box_SECRETKEYBYTES;
    const uint8_t *p = &in[0];
    if(in.size() >= crypto_box_PUBLICKEYBYTES) {
        mempcpy(
            publicKey->key,
            p,
            crypto_box_PUBLICKEYBYTES
        );
        p += crypto_box_PUBLICKEYBYTES;
    }
    if(in.size() >= pairSize) {
        mempcpy(
            secretKey->key,
            p,
            crypto_box_SECRETKEYBYTES
        );
        p += crypto_box_SECRETKEYBYTES;
    }
    if(in.size() > (pairSize + crypto_box_PUBLICKEYBYTES)) {
        mempcpy(
            outputKey->key,
            p,
            crypto_box_PUBLICKEYBYTES
        );
    }
}

std::ostream &operator<< (std::ostream &out, const MsgEncryptor::KeyTrio &t) {
    out << (const MsgEncryptor::KeyPair &)t;
    if(!t.outputKey) return out;
    out << "_"
        << *t.outputKey;
    return out;
}
