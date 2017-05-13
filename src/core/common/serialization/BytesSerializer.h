#ifndef GEO_NETWORK_CLIENT_BYTESSERIALIZER_H
#define GEO_NETWORK_CLIENT_BYTESSERIALIZER_H

#include "../NodeUUID.h"
#include "../memory/MemoryUtils.h"
#include "../exceptions/NotFoundError.h"

#include <vector>


class BytesSerializer {
protected:

    /**
     * This class implements interface for all derived serialization records classes.
     *
     * Serialization record contains data (often memory address and bytes count, but may contains other info),
     * that is usable for object serialization, and provides methods for accessing this data.
     *
     * Some serialization records may copy the object, or only store it's memory address.
     * The final behaviour is often defined by the object itself, and it's life cycle
     * (may it be copied, or not, is it temporary object, or would it be accessible until the collecting stage, etc.).
     *
     * This class only implements simple interface that returns pointer to the data,
     * that must be copied, and bytes count. The internal logic behind this methods are transferred to the derived classes.
     */
    class BaseSerializationRecord {
    public:
        BaseSerializationRecord(
            const size_t bytesCount)
            noexcept;

        virtual ~BaseSerializationRecord();

    size_t bytesCount() const
        noexcept;

    virtual void* pointer() const
        noexcept = 0;

    protected:
        const size_t mBytesCount;
    };


    /**
     * This record type is used for storing memory addresses of the objects,
     * that would be alive until the collecting stage. This record doesn't copy the object itself.
     *
     * Therefore, this record type must be used only for the object,
     * which is guaranteed to be alive when serialization would be started.
     */
    class DelayedRecord:
        public BaseSerializationRecord {

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


    /**
     * This record type is used for storing shared bytes buffers.
     * It is used for chaining of several serialization flows into the common one.
     * This record type allows collecting several bytes buffer, and then,
     * when serialization would be started - to merge them all into one common buffer,
     * and use only one additional memory allocation call.
     */
    class InlineRecord:
        public BaseSerializationRecord {

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

    /**
     * This record type is base for several derived record types,
     * all of which are designed to store primitive types (int8, int16, bool, etc).
     *
     * The main motivation for creting of this class is the next:
     * there is no reason to store the address to the object, when whole the object may be copied,
     * and this would use the same memory amount. Also, copying allows to keep temporary objects.
     */
    template <typename T>
    class OptimizedPrimitiveTypeInlineRecord:
        public BaseSerializationRecord {

    public:
        OptimizedPrimitiveTypeInlineRecord(
            T value)
            noexcept :

            BaseSerializationRecord(sizeof(value)),
            mValue(value) {}

        virtual void* pointer() const
            noexcept
        {
            return reinterpret_cast<void*>(
                const_cast<T*>(&mValue));
        }

    protected:
        const T mValue;
    };

    using InlineSizeTRecord = OptimizedPrimitiveTypeInlineRecord<size_t>;
    using InlineUInt16TRecord = OptimizedPrimitiveTypeInlineRecord<uint16_t>;
    using InlineUInt32TRecord = OptimizedPrimitiveTypeInlineRecord<uint32_t>;
    using InlineUInt64TRecord = OptimizedPrimitiveTypeInlineRecord<uint32_t>;
    using InlineByteRecord = OptimizedPrimitiveTypeInlineRecord<byte>;
    using InlineBoolRecord = OptimizedPrimitiveTypeInlineRecord<bool>;


public:
    void enqueue (
        const void *src,
        const size_t bytesCount)
        noexcept(false);

    void enqueue (
        const NodeUUID &nodeUUID)
        noexcept(false);

    void enqueue (
        BytesShared bytes,
        const size_t bytesCount)
        noexcept(false);

    void enqueue (
        pair<BytesShared, size_t> bytesAndSize)
        noexcept(false);

    void copy (
        const uint16_t value)
        noexcept(false);

    void copy (
        const uint32_t value)
        noexcept(false);

    void copy (
        const size_t value)
        noexcept(false);

    void copy (
        const bool value)
        noexcept(false);

    void copy (
        const byte value)
        noexcept(false);

    void copy (
        const NodeUUID &nodeUUID)
        noexcept(false);

    void copy (
        const void *src,
        const size_t bytesCount)
        noexcept(false);

    void copy (
        vector<byte> &bytes)
        noexcept(false);

    void merge (
        BytesSerializer &otherContainer)
        noexcept(false);

    const pair<BytesShared, size_t> collect() const
        noexcept(false);

protected:
    vector<BaseSerializationRecord*> mRecords;
};

#endif //GEO_NETWORK_CLIENT_BYTESSERIALIZER_H
