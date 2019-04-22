#ifndef GEO_NETWORK_CLIENT_MSGENCRYPTOR_H
#define GEO_NETWORK_CLIENT_MSGENCRYPTOR_H

#include "ByteEncryptor.h"
#include "../network/messages/Message.hpp"

class MsgEncryptor : public ByteEncryptor {
public:
    struct KeyTrio : KeyPair {
        KeyTrio();
        KeyTrio(const string &str);
        KeyTrio(const vector<uint8_t> &in) { deserialize(in); }
        void serialize(vector<uint8_t> &out) const;
        void deserialize(const vector<uint8_t> &in);
        PublicKeyShared contractorPublicKey = NULL;
    };

public:
    MsgEncryptor();
    using ByteEncryptor::ByteEncryptor;

public:
    static KeyTrio generateKeyTrio(
        const string &contractorPublicKey = "");

public:
    Buffer encrypt(Message::Shared message);
    Buffer decrypt(
        BytesShared buffer,
        const size_t count);
};

std::ostream &operator<< (std::ostream &out, const MsgEncryptor::KeyTrio &t);


#endif //GEO_NETWORK_CLIENT_MSGENCRYPTOR_H
