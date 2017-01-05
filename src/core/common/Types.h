#ifndef GEO_NETWORK_CLIENT_TYPES_H
#define GEO_NETWORK_CLIENT_TYPES_H

#include <cstdint>
#include <vector>
#include <memory>

typedef uint8_t byte;
typedef std::shared_ptr<byte> BytesShared;
typedef std::shared_ptr<const byte> ConstBytesShared;

enum TrustLineDirection {
    Incoming = 0,
    Outgoing = 1,
    Both     = 2,
};

typedef uint8_t SerializedTrustLineDirection;

#endif //GEO_NETWORK_CLIENT_TYPES_H
