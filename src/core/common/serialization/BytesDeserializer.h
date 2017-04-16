#ifndef GEO_NETWORK_CLIENT_BYTESDESERIALIZER_H
#define GEO_NETWORK_CLIENT_BYTESDESERIALIZER_H

#include "../Types.h"
#include "../NodeUUID.h"
#include "../memory/MemoryUtils.h"


class BytesDeserializer {
public:
    const BytesShared buffer;

public:
    BytesDeserializer(
        BytesShared buffer,
        size_t initialOffset=0);

    void copy (
        void *destination,
        const size_t bytesCount);

    void copyInto (
        const byte *b);

    void copyInto (
        const uint16_t *v);

    void copyInto (
        const uint32_t *v);

    void copyInto (
        const NodeUUID *nodeUUID);

protected:
    size_t mCurrentOffset;
};


#endif //GEO_NETWORK_CLIENT_BYTESDESERIALIZER_H
