#ifndef GEO_NETWORK_CLIENT_OBSERVINGMESSAGE_H
#define GEO_NETWORK_CLIENT_OBSERVINGMESSAGE_H

#include "../../../common/memory/MemoryUtils.h"

using namespace std;

class ObservingMessage {

public:
    typedef shared_ptr<ObservingMessage> Shared;
    typedef uint8_t SerializedType;
    typedef uint32_t MessageSize;

public:
    enum ProtocolVersion {
        Latest = 0,
    };

    enum MessageType {
        Observing_ClaimAppendRequest = 128,
        Observing_ParticipantsVotesAppendRequest = 64,
        Observing_ParticipantsVotesRequest = 68,
        Observing_TransactionsRequest = 192,
        Observing_BlockNumberRequest = 32,
    };

    virtual const MessageType typeID() const = 0;

    virtual BytesShared serializeToBytes() const
    {
        SerializedProtocolVersion kProtocolVersion = ProtocolVersion::Latest;
        const SerializedType kMessageType = typeID();
        const auto kMessageSize = (MessageSize)serializedSize();
        auto buffer = tryMalloc(kMessageSize);

        auto dataSize = kMessageSize - sizeof(MessageSize);
        memcpy(
            buffer.get(),
            &dataSize,
            sizeof(MessageSize));

        memcpy(
            buffer.get() + sizeof(MessageSize),
            &kProtocolVersion,
            sizeof(SerializedProtocolVersion));

        memcpy(
            buffer.get() + sizeof(MessageSize) + sizeof(SerializedProtocolVersion),
            &kMessageType,
            sizeof(SerializedType));

        return buffer;
    }

    virtual size_t serializedSize() const
    {
        return sizeof(MessageSize) +
               sizeof(SerializedProtocolVersion) +
               sizeof(SerializedType);
    }
};


#endif //GEO_NETWORK_CLIENT_OBSERVINGMESSAGE_H
