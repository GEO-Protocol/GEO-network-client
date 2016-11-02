#ifndef GEO_NETWORK_CLIENT_MESSAGE_H
#define GEO_NETWORK_CLIENT_MESSAGE_H

#include "../../common/exceptions/Exception.h"

#include <utility>
#include <stdint.h>
#include <memory>


class SerialisationError: public Exception {
    using Exception::Exception;
};

// Contains serialized messages: only bytes array and bytes counter.
// It's the message class responsibility for correct serialisation/deserialization.
// This class only holds the data in the memory and
// is used for efficient data transfer into the memory during methods calls.
class SerialisedMessage {
public:
    explicit SerialisedMessage(const uint8_t *bytes, const uint64_t bytesCount);
    ~SerialisedMessage();

    const uint64_t bytesCount() const;
    const uint8_t* bytes() const;

private:
    uint64_t mBytesCount;
    const uint8_t *mBytes;
};


// Base class for all messages in the system.
// All other messages are derived from this one.
class Message {
public:
    virtual std::shared_ptr<SerialisedMessage> serialize() const = 0;
};

#endif //GEO_NETWORK_CLIENT_MESSAGE_H
