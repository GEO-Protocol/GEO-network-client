#include "BytesSerializer.h"


BytesSerializer::BaseSerializationRecord::BaseSerializationRecord (
    const size_t bytesCount)
    noexcept :

    mBytesCount(bytesCount)
{
#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(bytesCount > 0);
#endif
}

size_t BytesSerializer::BaseSerializationRecord::bytesCount () const
    noexcept
{
    return mBytesCount;
}


BytesSerializer::DelayedRecord::DelayedRecord (
    const void* src,
    const size_t bytesCount)
    noexcept :

    BaseSerializationRecord(bytesCount),
    mSrc(src)
{}

void* BytesSerializer::DelayedRecord::pointer () const
    noexcept
{
    return const_cast<void*>(mSrc);
}


BytesSerializer::InlineRecord::InlineRecord (
    BytesShared bytes,
    const size_t bytesCount)
    noexcept :

    BaseSerializationRecord(bytesCount),
    mBytes(bytes)
{}

void* BytesSerializer::InlineRecord::pointer () const
    noexcept
{
    return mBytes.get();
}


void BytesSerializer::enqueue (
    const void *src,
    const size_t bytesCount)
    throw (bad_alloc&)
{
#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(bytesCount > 0);
#endif

    mRecords.push_back(
        new DelayedRecord(
            src,
            bytesCount));
}

void BytesSerializer::enqueue (
    const BytesShared bytes,
    const size_t bytesCount)
    throw (bad_alloc&)
{
#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(bytesCount > 0);
#endif

    mRecords.push_back(
        new InlineRecord(
            bytes,
            bytesCount));
}

void BytesSerializer::enqueue (
    pair<BytesShared, size_t> bytesAndSize)
    throw (bad_alloc &)
{
#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(bytesAndSize.second > 0);
#endif

    enqueue(
        bytesAndSize.first,
        bytesAndSize.second);
}

void BytesSerializer::enqueue (
    const uint16_t value)
    throw (bad_alloc&)
{
    mRecords.push_back(
        new InlineUInt16TRecord(value));
}

void BytesSerializer::enqueue (
    const uint32_t value)
    throw (bad_alloc&)
{
    mRecords.push_back(
        new InlineUInt32TRecord(value));
}

void BytesSerializer::enqueue (
    const size_t value)
    throw (bad_alloc&)
{
    mRecords.push_back(
        new InlineSizeTRecord(value));
}

void BytesSerializer::enqueue (
    const bool value)
    throw (bad_alloc&)
{
    mRecords.push_back(
        new InlineBoolRecord(value));
}

void BytesSerializer::enqueue (
    byte value)
    throw (bad_alloc&)
{
    mRecords.push_back(
        new InlineByteRecord(value));
}

void BytesSerializer::enqueue (
    const NodeUUID &nodeUUID)
    throw (bad_alloc&)
{
    enqueue(
        nodeUUID.data,
        NodeUUID::kBytesSize);
}

void BytesSerializer::copy (
    const void *src,
    const size_t bytesCount)
    throw (bad_alloc &)
{
#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(bytesCount > 0);
#endif

    auto buffer = tryMalloc(bytesCount);
    memcpy(
        buffer.get(),
        src,
        bytesCount);

    enqueue(
        buffer,
        bytesCount);
}

void BytesSerializer::copy (
    vector<byte> &bytes)
    throw (bad_alloc&)
{
#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(bytes.size() > 0);
#endif

    copy(
        bytes.data(),
        bytes.size());
}

void BytesSerializer::enqueue (BytesSerializer &otherContainer)
    throw (bad_alloc&)
{
#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(otherContainer.mRecords.size() > 0);
#endif

    mRecords.reserve(otherContainer.mRecords.size());
    for (const auto kRecord : otherContainer.mRecords)
        mRecords.push_back(kRecord);
}

const pair<BytesShared, size_t> BytesSerializer::collect () const
    throw (bad_alloc&)
{
    if (mRecords.size() == 0)
        throw NotFoundError(
            "BytesSerializer::data: "
                "there are no sources for collecting.");

    size_t totalBytesCount = 0;
    for (const auto kRecord : mRecords)
        totalBytesCount += kRecord->bytesCount();


    auto buffer = tryMalloc(totalBytesCount);
    auto currentBufferOffset = buffer.get();
    for (const auto kRecord : mRecords) {
        memcpy(
            currentBufferOffset,
            kRecord->pointer(),
            kRecord->bytesCount());

        currentBufferOffset += kRecord->bytesCount();
    }

    return make_pair(buffer, totalBytesCount);
}
