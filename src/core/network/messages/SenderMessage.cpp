#include "SenderMessage.h"


SenderMessage::SenderMessage(
    const NodeUUID &senderUUID)
    noexcept :

    senderUUID(senderUUID)
{}

SenderMessage::SenderMessage(
    BytesShared buffer)
    throw (bad_alloc)
{
    BytesDeserializer memory(
        buffer,
        Message::kOffsetToInheritedBytes());

    memory.copyIntoDespiteConst(&senderUUID);
}

pair<BytesShared, size_t> SenderMessage::serializeToBytes() const
    throw (bad_alloc)
{
    BytesSerializer serializer;

    serializer.enqueue(Message::serializeToBytes());
    serializer.enqueue(senderUUID);
    return serializer.collect();
}

const size_t SenderMessage::kOffsetToInheritedBytes() const
    noexcept
{
    static const auto kOffset =
        Message::kOffsetToInheritedBytes()
        + NodeUUID::kBytesSize;

    return kOffset;
}