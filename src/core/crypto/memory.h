#ifndef MEMORY_H
#define MEMORY_H

#include <sodium.h>
#include <sodium/core.h>

#include "../common/Types.h"


namespace crypto {
namespace memory {


class SecureSegment;

/**
 * @brief
 * SecureSegmentGuard is a helper class,
 * that is used for easy controlling of secure memory segment.
 *
 * On construction, it would unlock corresponding memory segment,
 * and would automatically lock it back on destruction.
 */
class SecureSegmentGuard {
public:

    /**
     * @brief
     * Unlocks memory segment for read/write operations.
     */
    SecureSegmentGuard(
        SecureSegment &segment)
        noexcept;

    /**
     * @brief
     * Locks memory segment for read/write operations.
     */
    ~SecureSegmentGuard()
        noexcept;

    /**
     * @returns address of first byte of allocated segment.
     */
    byte* address()
        const
        noexcept;

private:
    SecureSegment &mSegment;
};


/**
 * @brief
 * SecureSegment securely allocates and holds memory block.
 * Provides methods for locking/unlocking and accessing it for read/write operations.
 * See constructor documentation for more details about implementation.
 */
class SecureSegment {
public:
    friend class SecureSegmentGuard;

    /**
     * @brief
     * Securely allocates memory segment. Uses sodium_malloc() internally,
     * so the allocated region is placed at the end of a page boundary,
     * immediately followed by a guard page. As a result,
     * accessing memory past the end of the region will immediately terminate the application.
     *
     * A canary is also placed right before the returned pointer.
     * Modifications of this canary are detected when trying to free the allocated region,
     * and also cause the application to immediately terminate.
     *
     * An additional guard page is placed before this canary
     * to make it less likely for sensitive data to be accessible
     * when reading past the end of an unrelated region.
     *
     * The allocated region is filled with 0xdb bytes
     * in order to help catch bugs due to uninitialized data.
     *
     * In addition, sodium_mlock() is called on the region to help avoid it being swapped to disk.
     * On operating systems supporting MAP_NOCORE or MADV_DONTDUMP,
     * memory allocated this way will also not be part of core dumps.
     *
     * @throws "MemoryError" on allocation error.
     */
    SecureSegment(
        size_t bytesCount);

    /**
     * @brief
     * The wipe() function unlocks and deallocates memory allocated using constructor.
     * Calls sodium_free() internally, so prior to deallocation,
     * the canary is checked in order to detect possible buffer underflows
     * and terminate the process if required.
     *
     * Fills the memory region with zeros before the deallocation.
     */
    ~SecureSegment()
        noexcept;

    SecureSegmentGuard unlockAndInitGuard()
        const
        noexcept;

    /**
     * @returns current segment adress.
     */
    byte* address()
        const
        noexcept;

    /**
     * @returns current segment size.
     */
    size_t size()
        const
        noexcept;

    /**
     * @brief
     * Securely removes the content of the segment from the memory.
     */
    void wipeAndFree()
        noexcept;

private:
    byte *mAddress;
    size_t mSize;
};


}
}

#endif // MEMORY_H
