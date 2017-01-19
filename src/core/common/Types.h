#ifndef GEO_NETWORK_CLIENT_TYPES_H
#define GEO_NETWORK_CLIENT_TYPES_H


/*
 * Trust lines types
 */
#include <boost/multiprecision/cpp_int.hpp>
#include <memory>


#include <cstdint>
#include <vector>


using namespace std;


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
typedef shared_ptr<TrustLineAmount> SharedTrustLineAmount;
typedef shared_ptr<const TrustLineAmount> ConstSharedTrustLineAmount;

typedef boost::multiprecision::int256_t TrustLineBalance;
typedef shared_ptr<TrustLineBalance> SharedTrustLineBalance;
typedef shared_ptr<const TrustLineBalance> ConstSharedTrustLineBalance;

#endif //GEO_NETWORK_CLIENT_TYPES_H
