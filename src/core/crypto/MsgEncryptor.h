#ifndef GEO_NETWORK_CLIENT_MSGENCRYPTOR_H
#define GEO_NETWORK_CLIENT_MSGENCRYPTOR_H

#include "ByteEncryptor.h"

class Message;

class MsgEncryptor : public ByteEncryptor {
public:
    typedef shared_ptr<Message> MessageShared;

    struct KeyTrio : KeyPair {
        typedef std::shared_ptr<KeyTrio> Shared;
        KeyTrio();
        explicit KeyTrio(const string &str);
        explicit KeyTrio(vector<byte> &in);
        void serialize(vector<byte> &out) const;
        void deserialize(vector<byte> &in);
        PublicKey::Shared contractorPublicKey = nullptr;
    };

public:
    using ByteEncryptor::ByteEncryptor;

public:
    static KeyTrio::Shared generateKeyTrio(
        const string &contractorPublicKey = "");

public:
    Buffer encrypt(MessageShared message);
    Buffer decrypt(
        BytesShared buffer,
        const size_t count);
};

std::ostream &operator<< (std::ostream &out, const MsgEncryptor::KeyTrio &t);


#endif //GEO_NETWORK_CLIENT_MSGENCRYPTOR_H
