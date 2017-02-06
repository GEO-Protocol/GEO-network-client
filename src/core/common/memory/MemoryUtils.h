#ifndef GEO_NETWORK_CLIENT_MEMORYUTILS_H
#define GEO_NETWORK_CLIENT_MEMORYUTILS_H

#include "../Types.h"

#include <cstdint>
#include <memory>
#include <malloc.h>

class MemoryUtils {

    static BytesShared tryMalloc(
        size_t bytesCount);

    static BytesShared tryCalloc(
        size_t bytesCount);
};


#endif //GEO_NETWORK_CLIENT_MEMORYUTILS_H
