#include "memory.h"


BytesShared tryMalloc(
    size_t bytesCount) {

    BytesShared buffer(
        (byte *)malloc(bytesCount),
        free);

    if (buffer == nullptr) {
        throw std::bad_alloc();
    }

    return buffer;
}

BytesShared tryCalloc(
    size_t bytesCount) {

    BytesShared buffer(
        (byte *)calloc(
            bytesCount,
            sizeof(byte)),
        free);

    if (buffer == nullptr) {
        throw std::bad_alloc();
    }

    return buffer;
}
