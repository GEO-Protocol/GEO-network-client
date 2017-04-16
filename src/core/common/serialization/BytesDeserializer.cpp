#include "BytesDeserializer.h"


BytesDeserializer::BytesDeserializer (
    BytesShared buffer,
    size_t initialOffset):

    buffer(buffer),
    mCurrentOffset(initialOffset)
{}

void BytesDeserializer::copy (
    void *destination,
    const size_t bytesCount)
{
    memcpy(
        destination,
        buffer.get() + mCurrentOffset,
        bytesCount);

    mCurrentOffset += bytesCount;
}

void BytesDeserializer::copyInto (
    const byte *b)
{
    copy(
        b,
        sizeof(b));
}

void BytesDeserializer::copyInto (
    const uint16_t *v)
{
    copy(
        v,
        sizeof(v));
}

void BytesDeserializer::copyInto (
    const uint32_t *v)
{
    copy(
        v,
        sizeof(v));
}


void BytesDeserializer::copyInto (
    const NodeUUID *nodeUUID) {

    copy(
        nodeUUID->data,
        NodeUUID::kBytesSize);
}
