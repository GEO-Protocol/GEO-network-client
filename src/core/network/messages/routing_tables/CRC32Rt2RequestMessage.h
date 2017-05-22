#ifndef GEO_NETWORK_CLIENT_REQUESTROUTGHTINGTABLESCRC32REQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_REQUESTROUTGHTINGTABLESCRC32REQUESTMESSAGE_H


#include "../SenderMessage.h"


class CRC32Rt2RequestMessage :
    public SenderMessage {

public:
    typedef shared_ptr<CRC32Rt2RequestMessage> Shared;

public:
    using SenderMessage::SenderMessage;

public:

    pair<BytesShared, size_t> serializeToBytes() const
    throw (bad_alloc);

    virtual const MessageType typeID() const
    noexcept;
};


#endif //GEO_NETWORK_CLIENT_REQUESTROUTGHTINGTABLESCRC32REQUESTMESSAGE_H
