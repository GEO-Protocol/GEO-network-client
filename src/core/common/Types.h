#ifndef GEO_NETWORK_CLIENT_TYPES_H
#define GEO_NETWORK_CLIENT_TYPES_H

#include <cstdint>
#include <vector>


typedef uint8_t byte;

enum TrustLineDirection {
    Unspecified = 0,

    Incoming = 1,
    Outgoing = 2,
    Both     = 3,
};
typedef uint8_t SerializedTrustLineDirection;

#endif //GEO_NETWORK_CLIENT_TYPES_H
