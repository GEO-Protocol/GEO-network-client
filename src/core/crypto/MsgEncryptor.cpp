#include "MsgEncryptor.h"

MsgEncryptor::MsgEncryptor() :
    ByteEncryptor(
        defaultKeyPair().publicKey,
        defaultKeyPair().secretKey)
{}

ByteEncryptor::Buffer MsgEncryptor::encrypt(Message::Shared message) {
    auto bytesAndBytesCount = message->serializeToBytes();

    auto pair = ByteEncryptor::encrypt(
        bytesAndBytesCount.first.get() + Message::UnencryptedHeaderSize,
        bytesAndBytesCount.second - Message::UnencryptedHeaderSize,
        Message::UnencryptedHeaderSize
    );
    memcpy(
        pair.first.get(),
        bytesAndBytesCount.first.get(),
        Message::UnencryptedHeaderSize);
    return pair;
}

ByteEncryptor::Buffer MsgEncryptor::decrypt(BytesShared buffer, const size_t count) {
    auto pair = ByteEncryptor::decrypt(
        buffer.get() + Message::UnencryptedHeaderSize,
        count - Message::UnencryptedHeaderSize,
        Message::UnencryptedHeaderSize
    );
    memcpy(
        pair.first.get(),
        buffer.get(),
        Message::UnencryptedHeaderSize);
    return pair;
}
