#ifndef GEO_NETWORK_CLIENT_MSGENCRYPTOR_H
#define GEO_NETWORK_CLIENT_MSGENCRYPTOR_H

#include "ByteEncryptor.h"
#include "../network/messages/Message.hpp"

class MsgEncryptor : public ByteEncryptor {
public:
    struct KeyTrio : KeyPair {
        typedef std::shared_ptr<KeyTrio> Shared;
        KeyTrio();
        KeyTrio(const string &str);
        KeyTrio(const vector<byte> &in) { deserialize(in); }
        void serialize(vector<byte> &out) const;
        void deserialize(const vector<byte> &in);
        PublicKey::Shared contractorPublicKey = nullptr;
    };

public:
    MsgEncryptor();
    using ByteEncryptor::ByteEncryptor;

public:
    static KeyTrio::Shared generateKeyTrio(
        const string &contractorPublicKey = "");

public:
    Buffer encrypt(Message::Shared message);
    Buffer decrypt(
        BytesShared buffer,
        const size_t count);
};

std::ostream &operator<< (std::ostream &out, const MsgEncryptor::KeyTrio &t);


#endif //GEO_NETWORK_CLIENT_MSGENCRYPTOR_H
