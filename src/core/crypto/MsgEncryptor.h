#ifndef GEO_NETWORK_CLIENT_MSGENCRYPTOR_H
#define GEO_NETWORK_CLIENT_MSGENCRYPTOR_H

#include "ByteEncryptor.h"
#include "../network/messages/Message.hpp"

class MsgEncryptor : public ByteEncryptor {
public:
    MsgEncryptor();

public:
    Buffer encrypt(Message::Shared message);
    Buffer decrypt(
        BytesShared buffer,
        const size_t count);
};


#endif //GEO_NETWORK_CLIENT_MSGENCRYPTOR_H
