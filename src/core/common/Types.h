#ifndef GEO_NETWORK_CLIENT_TYPES_H
#define GEO_NETWORK_CLIENT_TYPES_H


/*
 * Trust lines types
 */
#include <boost/multiprecision/cpp_int.hpp>


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



/*
 * Trust lines types
 */
typedef boost::multiprecision::checked_uint256_t TrustLineAmount;
typedef boost::multiprecision::checked_uint256_t TrustLineBlockAmount;
typedef boost::multiprecision::int256_t TrustLineBalance;

#endif //GEO_NETWORK_CLIENT_TYPES_H
