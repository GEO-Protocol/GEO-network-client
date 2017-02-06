#include "MemoryUtils.h"

BytesShared MemoryUtils::tryMalloc(
    size_t bytesCount) {

    BytesShared bytesShared(
        (byte *)malloc(bytesCount),
        free
    );

    if (bytesShared == nullptr) {
        throw std::bad_alloc();
    }

    return bytesShared;
}

BytesShared MemoryUtils::tryCalloc(
    size_t bytesCount) {

    BytesShared bytesShared(
        (byte *)calloc(
            bytesCount,
            sizeof(byte)
        ),
        free
    );

    if (bytesShared == nullptr) {
        throw std::bad_alloc();
    }

    return bytesShared;
}
