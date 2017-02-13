#ifndef GEO_NETWORK_CLIENT_TYPES_H
#define GEO_NETWORK_CLIENT_TYPES_H

#include <boost/multiprecision/cpp_int.hpp>

#include <memory>
#include <cstdint>

using namespace std;

/*
 * Byte
 */
typedef uint8_t byte;
typedef std::shared_ptr<byte> BytesShared;
typedef std::shared_ptr<const byte> ConstBytesShared;

/*
 * Trust lines types
 */
namespace multiprecision = boost::multiprecision;
typedef multiprecision::checked_uint256_t TrustLineAmount;
typedef shared_ptr<TrustLineAmount> SharedTrustLineAmount;
typedef shared_ptr<const TrustLineAmount> ConstSharedTrustLineAmount;
const size_t kTrustLineAmountBytesCount = 32;

typedef multiprecision::int256_t TrustLineBalance;
typedef shared_ptr<TrustLineBalance> SharedTrustLineBalance;
typedef shared_ptr<const TrustLineBalance> ConstSharedTrustLineBalance;
const size_t kTrustLineBalanceBytesCount = 32;

enum BalanceRange {
    Negative = 1,
    EqualsZero = 2,
    Positive = 3
};

enum TrustState {
    Active = 1,
    Suspended = 2
};

enum TrustLineDirection {
    Nowhere = 0,
    Incoming,
    Outgoing,
    Both,
};
typedef uint8_t SerializedTrustLineDirection;

#endif //GEO_NETWORK_CLIENT_TYPES_H
