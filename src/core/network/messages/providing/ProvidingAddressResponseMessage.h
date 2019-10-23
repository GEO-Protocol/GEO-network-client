#ifndef GEO_NETWORK_CLIENT_PROVIDINGADDRESSRESPONSEMESSAGE_H
#define GEO_NETWORK_CLIENT_PROVIDINGADDRESSRESPONSEMESSAGE_H

#include "../Message.hpp"

class ProvidingAddressResponseMessage : public Message {

public:
    typedef shared_ptr<ProvidingAddressResponseMessage> Shared;

public:
    ProvidingAddressResponseMessage(
        BytesShared buffer);

    BytesShared buffer() const;

    const MessageType typeID() const override;

public:
    static const size_t UnencryptedHeaderSize =
            Message::UnencryptedHeaderSize + sizeof(Message::SerializedType);

private:
    BytesShared mBuffer;
};


#endif //GEO_NETWORK_CLIENT_PROVIDINGADDRESSRESPONSEMESSAGE_H
