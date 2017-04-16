#ifndef GEO_NETWORK_CLIENT_BYTESSERIALIZER_H
#define GEO_NETWORK_CLIENT_BYTESSERIALIZER_H

#include "../NodeUUID.h"
#include "../memory/MemoryUtils.h"
#include "../exceptions/NotFoundError.h"

#include <vector>


class BytesSerializer {
protected:
    class BaseSerializationRecord {
    public:
        BaseSerializationRecord(
            const size_t bytesCount)
            noexcept;

    size_t bytesCount() const
        noexcept;

    virtual void* pointer() const
        noexcept = 0;

    protected:
        const size_t mBytesCount;
    };


    class DelayedRecord {
    public:
        DelayedRecord(
            const void *src,
            const size_t bytesCount)
            noexcept;

        virtual void* pointer() const
            noexcept;

    protected:
        const void *mSrc;
    };


    class InlineRecord {
    public:
        InlineRecord(
            BytesShared bytes,
            const size_t bytesCount)
            noexcept;

    virtual void* pointer() const
        noexcept;

    protected:
        const BytesShared mBytes;
    };

    template <typename T>
    class OptimizedPrimitiveTypeInlineRecord {
    public:
        OptimizedPrimitiveTypeInlineRecord(
            T value)
            noexcept :

            mValue(value) {}

        virtual void* pointer() const
            noexcept {

            return (void*)&mValue;
        };

    protected:
        const T mValue;
    };

    typedef OptimizedPrimitiveTypeInlineRecord<size_t> InlineSizeTRecord;
    typedef OptimizedPrimitiveTypeInlineRecord<uint16_t> InlineUInt16TRecord;
    typedef OptimizedPrimitiveTypeInlineRecord<uint32_t> InlineUInt32TRecord;
    typedef OptimizedPrimitiveTypeInlineRecord<byte> InlineByteRecord;
    typedef OptimizedPrimitiveTypeInlineRecord<bool> InlineBoolRecord;


public:
    void enqueue (
        const void *src,
        const size_t bytesCount)
        throw (bad_alloc &);

    void enqueue (
        BytesShared bytes,
        const size_t bytesCount)
        throw (bad_alloc &);

    void enqueue (
        pair<BytesShared, size_t> bytesAndSize)
        throw (bad_alloc &);

    void enqueue (
        const uint16_t value)
        throw (bad_alloc &);

    void enqueue (
        const size_t value)
        throw (bad_alloc &);

    void enqueue (
        const bool value)
        throw (bad_alloc &);

    void enqueue (
        const byte value)
        throw (bad_alloc &);

    void enqueue (
        const NodeUUID &nodeUUID)
        throw (bad_alloc &);

    void copy (
        const void *src,
        const size_t bytesCount)
        throw (bad_alloc &);

    void copy (
        vector<byte> &bytes)
        throw (bad_alloc &);

    void enqueue (
        BytesSerializer &otherContainer)
        throw (bad_alloc &);

    const pair<BytesShared, size_t> collect() const
        throw (bad_alloc& );

protected:
    vector<BaseSerializationRecord*> mRecords;
};

#endif //GEO_NETWORK_CLIENT_BYTESSERIALIZER_H
