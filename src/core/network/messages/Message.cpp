#include "Message.h"

// This constructor is used for serialisation purposes.
SerialisedMessage::SerialisedMessage(const uint8_t *messageBytes,
                                     const uint16_t messageBytesCount,
                                     const Message::MessageTypeID messageTypeID){
    // Arguments validation
    if (messageBytes == nullptr) {
        throw SerialisationError("Bytes array can't be empty!");
    }

    if (messageBytesCount == 0) {
        throw SerialisationError("Bytes count can't be zero!");
    }

    initialiseInternalSerialisationBuffer(
        messageBytes, messageBytesCount, messageTypeID);
}

// This constructor is used for restoring from raw bytes sequences.
//
// NOTE: this instance will take ownership on *rawMessageBytes;
// It should not be freed after calling this constructor;
SerialisedMessage::SerialisedMessage(const uint8_t *rawMessageBytes,
                                     const uint16_t rawMessageBytesCount){
    // Arguments validation
    if (rawMessageBytes == nullptr) {
        throw SerialisationError("Bytes array can't be empty!");
    }

    if (rawMessageBytesCount == 0) {
        throw SerialisationError("Bytes count can't be zero!");
    }

    mBytesCount = rawMessageBytesCount;
    mBytesBuffer = (uint8_t*)rawMessageBytes;
}

SerialisedMessage::~SerialisedMessage() {
    free(mBytesBuffer);
}

const uint64_t SerialisedMessage::bytesCount() const {
    return mBytesCount;
}

const uint8_t *SerialisedMessage::bytes() const {
    return mBytesBuffer;
}

// Creates and returns buffer, that contains message and standard header.
void SerialisedMessage::initialiseInternalSerialisationBuffer(const uint8_t *messageBytes,
                                                              const uint16_t messageBytesCount,
                                                              const Message::MessageTypeID messageTypeID) {

    static const uint8_t StandardMessageHeaderSize =
        sizeof(uint16_t) + // message size (2B)
        sizeof(uint32_t) + // crc32 checksum (4B)
        sizeof(uint8_t);   // message type id (1B)

    static const uint8_t sizeHeaderOffset = 0;
    static const uint8_t crc32HeaderOffset = sizeof(uint16_t);
    static const uint8_t messageTypeHeaderOffset = crc32HeaderOffset + sizeof(uint32_t);
    static const uint8_t messageContentOffset = messageTypeHeaderOffset + sizeof(uint8_t);

    // Buffer initialisation
    mBytesBuffer = (uint8_t*)malloc(StandardMessageHeaderSize + messageBytesCount);
    if (mBytesBuffer == nullptr) {
        throw MemoryError("Can't allocate memory for new message buffer.");
    }

    // Message size header.
    // This header is equal to message body and all the headers.
    new (mBytesBuffer + sizeHeaderOffset)
        uint16_t(messageBytesCount + StandardMessageHeaderSize);

    // CRC32 checksum
    new (mBytesBuffer + crc32HeaderOffset)
        uint32_t (0); // todo: add realisation

    // Message type ID
    new (mBytesBuffer + messageTypeHeaderOffset)
        uint8_t(messageTypeID);

    // Copying message bytes into new buffer;
    memcpy(mBytesBuffer + messageContentOffset, messageBytes, messageBytesCount);
}
