#include "Message.h"

// NOTE: This instance will take ownership on the "bytes".
SerialisedMessage::SerialisedMessage(const uint8_t *bytes, const uint64_t bytesCount)
    :mBytes(bytes), mBytesCount(bytesCount) {

    // Arguments validation
    if (bytes == nullptr) {
        throw SerialisationError("Bytes array can't be empty!");
    }

    if (bytesCount == 0) {
        throw SerialisationError("Bytes count can't be zero!");
    }
}

SerialisedMessage::~SerialisedMessage() {
    delete mBytes;
}

const uint64_t SerialisedMessage::bytesCount() const {
    return mBytesCount;
}

const uint8_t *SerialisedMessage::bytes() const {
    return mBytes;
}
