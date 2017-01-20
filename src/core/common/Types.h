#ifndef GEO_NETWORK_CLIENT_TYPES_H
#define GEO_NETWORK_CLIENT_TYPES_H

#include <cstdint>
#include <memory>

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <boost/multiprecision/cpp_int.hpp>

typedef uint8_t byte;
typedef std::shared_ptr<byte> BytesShared;
typedef std::shared_ptr<const byte> ConstBytesShared;

namespace posix = boost::posix_time;
typedef posix::ptime Timestamp;
typedef posix::time_duration Duration;
typedef uint64_t MicrosecondsTimestamp;

namespace multiprecision = boost::multiprecision;
typedef multiprecision::checked_uint256_t TrustLineAmount;
typedef multiprecision::int256_t TrustLineBalance;

enum TrustLineDirection {
    Incoming = 0,
    Outgoing = 1,
    Both     = 2,
    Nowhere  = 3
};
typedef uint8_t SerializedTrustLineDirection;

enum BalanceRange {
    Negative = 1,
    EqualsZero = 2,
    Positive = 3
};

enum TrustState {
    Active = 1,
    Suspended = 2,
    PendingSuspend = 3
};

#endif //GEO_NETWORK_CLIENT_TYPES_H
