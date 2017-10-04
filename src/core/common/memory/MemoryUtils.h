#ifndef GEO_NETWORK_CLIENT_MEMORYUTILS_H
#define GEO_NETWORK_CLIENT_MEMORYUTILS_H

#include "../Types.h"

#include <cstdint>
#include <memory>

#ifdef MAC_OS
#include <stdlib.h>
#endif

#ifdef LINUX
#include <malloc.h>
#endif


using namespace std;


inline BytesShared tryMalloc(
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

inline BytesShared tryCalloc(
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
#endif //GEO_NETWORK_CLIENT_MEMORYUTILS_H
