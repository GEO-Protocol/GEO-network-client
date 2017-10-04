#include "BytesDeserializer.h"


BytesDeserializer::BytesDeserializer (
    BytesShared buffer,
    size_t initialOffset)
    noexcept :

    buffer(buffer),
    mCurrentOffset(initialOffset)
{}

void BytesDeserializer::copyInto (
    byte *b)
    noexcept
{
    copyInto(
        (void*)b,
        sizeof(*b));
}

void BytesDeserializer::copyInto (
    uint16_t *v)
    noexcept
{
    copyInto(
        (void*)v,
        sizeof(*v));
}

void BytesDeserializer::copyInto (
    uint32_t *v)
    noexcept
{
    copyInto(
        (void*)v,
        sizeof(*v));
}

void BytesDeserializer::copyInto (
    NodeUUID *nodeUUID)
    noexcept
{
    copyInto(
        nodeUUID->data,
        NodeUUID::kBytesSize);
}

void BytesDeserializer::copyInto (
    void *destination,
    const size_t bytesCount)
    noexcept
{

    memcpy(
        destination,
        buffer.get() + mCurrentOffset,
        bytesCount);

    mCurrentOffset += bytesCount;
}

void BytesDeserializer::copyIntoDespiteConst (
    const byte *b)
    noexcept
{
    copyInto(const_cast<byte*>(b));
}

void BytesDeserializer::copyIntoDespiteConst (
    const uint16_t *v)
    noexcept
{
    copyInto(const_cast<uint16_t*>(v));
}

void BytesDeserializer::copyIntoDespiteConst (
    const uint32_t *v)
    noexcept
{
    copyInto(const_cast<uint32_t*>(v));
}

void BytesDeserializer::copyIntoDespiteConst (
    const NodeUUID *nodeUUID)
    noexcept
{
    copyInto(const_cast<NodeUUID*>(nodeUUID));
}
