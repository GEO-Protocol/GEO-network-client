#include "BytesSerializer.h"


/**
 * @param bytesCount - specifies how many bytes must be reserved in to the memory
 *      for the object serialzation.
 */
BytesSerializer::BaseSerializationRecord::BaseSerializationRecord (
    const size_t bytesCount)
    noexcept :

    mBytesCount(bytesCount)
{
#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(bytesCount > 0);
#endif
}

BytesSerializer::BaseSerializationRecord::~BaseSerializationRecord()
{
    // Empty virtual descructor is needed, because class contains virtual methods.
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

/**
 * Enqueues the address of the "src" for the serialization in the future.
 *
 * @throws bad_alloc;
 */
void BytesSerializer::enqueue (
    const void *src,
    const size_t bytesCount)
    noexcept(false)
{
#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(bytesCount > 0);
#endif

    mRecords.push_back(
        new DelayedRecord(
            src,
            bytesCount));
}

/**
 * Copies the shared pointer to this serializer.
 * SAFE FOR USAGE WITH TEMPORARY copy of shared pointer.
 *
 * @throws bad_alloc;
 */
void BytesSerializer::enqueue (
    const BytesShared bytes,
    const size_t bytesCount)
    noexcept(false)
{
#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(bytesCount > 0);
#endif

    mRecords.push_back(
        new InlineRecord(
            bytes,
            bytesCount));
}

/**
 * Copies the shared pointer to this serializer.
 * This is a shortcut method for fast enqueuing of the results
 * of the serialization methods of other objects.
 *
 * SAFE FOR USAGE WITH TEMPORARY copy of shared pointer.
 *
 * @throws bad_alloc;
 */
void BytesSerializer::enqueue (
    pair<BytesShared, size_t> bytesAndSize)
    noexcept(false)
{
#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(bytesAndSize.second > 0);
#endif

    enqueue(
        bytesAndSize.first,
        bytesAndSize.second);
}

/**
 * SAFE FOR USAGE WITH TEMPORARY VALUE.
 *
 * @throws bad_alloc;
 */
void BytesSerializer::copy (
    const uint16_t value)
    noexcept(false)
{
    mRecords.push_back(
        new InlineUInt16TRecord(value));
}

/**
 * SAFE FOR USAGE WITH TEMPORARY VALUE.
 *
 * @throws bad_alloc;
 */
void BytesSerializer::copy (
    const uint32_t value)
    noexcept(false)
{
    mRecords.push_back(
        new InlineUInt32TRecord(value));
}

/**
 * SAFE FOR USAGE WITH TEMPORARY VALUE.
 *
 * @throws bad_alloc;
 */
void BytesSerializer::copy (
    const size_t value)
    noexcept(false)
{
    mRecords.push_back(
        new InlineSizeTRecord(value));
}

/**
 * SAFE FOR USAGE WITH TEMPORARY VALUE.
 *
 * @throws bad_alloc;
 */
void BytesSerializer::copy (
    const bool value)
    noexcept(false)
{
    mRecords.push_back(
        new InlineBoolRecord(value));
}

/**
 * SAFE FOR USAGE WITH TEMPORARY VALUE.
 *
 * @throws bad_alloc;
 */
void BytesSerializer::copy(
    const NodeUUID &nodeUUID)
    noexcept(false)
{
    copy(
        nodeUUID.data,
        NodeUUID::kBytesSize);
}

/**
 * SAFE FOR USAGE WITH TEMPORARY VALUE.
 *
 * @throws bad_alloc;
 */
void BytesSerializer::copy (
    byte value)
    noexcept(false)
{
    mRecords.push_back(
        new InlineByteRecord(value));
}

/**
 * @throws bad_alloc;
 */
void BytesSerializer::enqueue (
    const NodeUUID &nodeUUID)
    noexcept(false)
{
    enqueue(
        nodeUUID.data,
        NodeUUID::kBytesSize);
}

/**
 * @throws bad_alloc;
 */
void BytesSerializer::copy (
    const void *src,
    const size_t bytesCount)
    noexcept(false)
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

/**
 * @throws bad_alloc;
 */
void BytesSerializer::copy (
    vector<byte> &bytes)
    noexcept(false)
{
#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(bytes.size() > 0);
#endif

    copy(
        bytes.data(),
        bytes.size());
}

/**
 * Chains other serializer into this one,
 * by copying its records into internal storage.
 *
 * @throws bad_alloc;
 */
void BytesSerializer::merge (
    BytesSerializer &otherContainer)
    noexcept(false)
{
#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(otherContainer.mRecords.size() > 0);
#endif

    if (otherContainer.mRecords.size() > 0) {
        mRecords.reserve(otherContainer.mRecords.size());
        for (const auto kRecord : otherContainer.mRecords)
            mRecords.push_back(kRecord);
    }
}

/**
 * Calculates total bytes count needed for all the data,
 * allocates memory segment of the needed length and populates it by the data
 * from all collected internal records.
 *
 * @returns shared buffer with populated data.
 *
 * @throws bad_alloc;
 */
const pair<BytesShared, size_t> BytesSerializer::collect () const
    noexcept(false)
{
    if (mRecords.size() == 0)
        throw NotFoundError(
            "BytesSerializer::collect: "
            "there are no sources for collecting.");

    size_t totalBytesCount = 0;
    for (const auto kRecord : mRecords) {
        totalBytesCount += kRecord->bytesCount();
    }


    auto buffer = tryMalloc(totalBytesCount);
    auto currentBufferOffset = buffer.get();
    for (const auto kRecord : mRecords) {
        memcpy(
            currentBufferOffset,
            kRecord->pointer(),
            kRecord->bytesCount());

        currentBufferOffset += kRecord->bytesCount();

        delete kRecord;
    }

    return make_pair(
        buffer,
        totalBytesCount);
}
