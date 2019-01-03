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
typedef uint32_t TrustLineID;
typedef uint32_t KeyNumber;
typedef uint32_t KeysCount;
typedef uint32_t AuditNumber;
const size_t kTrustLineAmountBytesCount = 32;

typedef multiprecision::int256_t TrustLineBalance;
typedef shared_ptr<TrustLineBalance> SharedTrustLineBalance;
typedef shared_ptr<const TrustLineBalance> ConstSharedTrustLineBalance;

const size_t kTrustLineBalanceBytesCount = 32;
const size_t kTrustLineBalanceSerializeBytesCount = 33;

static const constexpr char kCommandsSeparator = '\n';
static const constexpr char kTokensSeparator = '\t';
static const constexpr char kAddressTypeSeparator = '-';
static const constexpr char kAddressSeparator = ':';

typedef uint16_t SerializedRecordsCount;
typedef SerializedRecordsCount SerializedRecordNumber;

typedef uint16_t SerializedResponseCode;

// payments
typedef uint16_t PathID;
typedef uint8_t SerializedPathLengthSize;
typedef SerializedPathLengthSize SerializedPositionInPath;
typedef uint16_t PaymentNodeID;

typedef uint16_t ConfirmationID;

//equivalents
typedef uint32_t SerializedEquivalent;
typedef byte SerializedProtocolVersion;

typedef uint32_t ContractorID;

#endif //GEO_NETWORK_CLIENT_TYPES_H
