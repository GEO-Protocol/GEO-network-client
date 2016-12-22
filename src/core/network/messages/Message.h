#ifndef GEO_NETWORK_CLIENT_MESSAGE_H
#define GEO_NETWORK_CLIENT_MESSAGE_H

#include "../../common/exceptions/Exception.h"
#include "../../common/exceptions/MemoryError.h"

#include <stdint.h>
#include <malloc.h>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <utility>


class SerialisationError: public Exception {
    using Exception::Exception;
};


// Base class for all messages in the system.
// All other messages are derived from this one.
class SerialisedMessage;
class Message {
public:
    typedef shared_ptr<Message> Shared;
public:
    enum MessageTypeID {
        // System messages.
        // Used for low level system logic:
        // for example, message processing reports, etc.
        ProcessingReportMessage = 1,

        // Test messages
        TestSimpleMessage, TestLongMessage,

        // User space messages
    };

public:
    virtual std::shared_ptr<SerialisedMessage> serialize() const = 0;
    virtual const MessageTypeID typeID() const = 0;
};

// Contains serialized messages: only bytes array and bytes counter.
// It's the message class responsibility for correct serialisation/deserialization.
// This class only holds the data in the memory and
// is used for efficient data transfer into the memory during methods calls.
class SerialisedMessage {
public:
    explicit SerialisedMessage(const uint8_t *rawMessageBytes,
                               const uint16_t rawMessageBytesCount);

    explicit SerialisedMessage(const uint8_t *messageBytes,
                               const uint16_t messageBytesCount,
                               const Message::MessageTypeID messageTypeID);

    ~SerialisedMessage();

    const uint64_t bytesCount() const;
    const uint8_t* bytes() const;

private:
    void initialiseInternalSerialisationBuffer(const uint8_t *messageBytes,
                                               const uint16_t messageBytesCount,
                                               const Message::MessageTypeID messageTypeID);

private:
    uint64_t mBytesCount;
    uint8_t *mBytesBuffer;
};

#endif //GEO_NETWORK_CLIENT_MESSAGE_H
