#include "SenderMessage.h"


SenderMessage::SenderMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID)
    noexcept :
    EquivalentMessage(
        equivalent),
    senderUUID(senderUUID)
{}

SenderMessage::SenderMessage(
    BytesShared buffer)
    noexcept:
    EquivalentMessage(buffer)
{
    memcpy(
        const_cast<NodeUUID*>(&senderUUID),
        buffer.get() + EquivalentMessage::kOffsetToInheritedBytes(),
        NodeUUID::kBytesSize);
}

/**
 * @throws bad_alloc;
 */
pair<BytesShared, size_t> SenderMessage::serializeToBytes() const
    noexcept(false)
{
    BytesSerializer serializer;

    serializer.enqueue(EquivalentMessage::serializeToBytes());
    serializer.enqueue(senderUUID);
    return serializer.collect();
}

const size_t SenderMessage::kOffsetToInheritedBytes() const
    noexcept
{
    static const auto kOffset =
        EquivalentMessage::kOffsetToInheritedBytes()
        + NodeUUID::kBytesSize;
    return kOffset;
}
