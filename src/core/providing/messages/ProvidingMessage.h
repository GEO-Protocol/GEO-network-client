#ifndef GEO_NETWORK_CLIENT_PROVIDINGMESSAGE_H
#define GEO_NETWORK_CLIENT_PROVIDINGMESSAGE_H

#include "../../common/memory/MemoryUtils.h"

class ProvidingMessage {

public:
    typedef shared_ptr<ProvidingMessage> Shared;
    typedef uint8_t SerializedType;
    typedef uint32_t MessageSize;

public:
    enum ProtocolVersion {
        Latest = 0,
    };

    enum MessageType {
        Providing_Ping = 1,
        Providing_Pong = 2,
    };

    ProvidingMessage(
        ProviderParticipantID participantID);

    virtual const MessageType typeID() const = 0;

    virtual BytesShared serializeToBytes() const;

    virtual size_t serializedSize() const;

private:
    ProviderParticipantID mParticipantID;
};


#endif //GEO_NETWORK_CLIENT_PROVIDINGMESSAGE_H
