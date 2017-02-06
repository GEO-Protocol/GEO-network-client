#ifndef GEO_NETWORK_CLIENT_TYPES_H
#define GEO_NETWORK_CLIENT_TYPES_H


/*
 * Trust lines types
 */
#include <boost/multiprecision/cpp_int.hpp>
#include <memory>


#include <cstdint>
#include <memory>

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <boost/multiprecision/cpp_int.hpp>

using namespace std;


typedef uint8_t byte;
typedef std::shared_ptr<byte> BytesShared;
typedef std::shared_ptr<const byte> ConstBytesShared;


/*
 * Timestamps
 */
namespace posix = boost::posix_time;
typedef posix::ptime Timestamp;
typedef posix::time_duration Duration;
typedef uint64_t MicrosecondsTimestamp;


/*
 * Trust lines types
 */
namespace multiprecision = boost::multiprecision;
typedef multiprecision::checked_uint256_t TrustLineAmount;
typedef shared_ptr<TrustLineAmount> SharedTrustLineAmount;
typedef shared_ptr<const TrustLineAmount> ConstSharedTrustLineAmount;
const byte kTrustLineAmountBytesCount = 32;

typedef multiprecision::int256_t TrustLineBalance;
typedef shared_ptr<TrustLineBalance> SharedTrustLineBalance;
typedef shared_ptr<const TrustLineBalance> ConstSharedTrustLineBalance;
const byte kTrustLineBalanceBytesCount = 32;

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
    // System cpecific
    // (there are no trust lines of this type in the system).
    Nowhere = 0,
    Incoming,
    Outgoing,
    Both,
};
typedef uint8_t SerializedTrustLineDirection;

#endif //GEO_NETWORK_CLIENT_TYPES_H
