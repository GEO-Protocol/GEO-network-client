#ifndef GEO_NETWORK_CLIENT_CONSTANTS_H
#define GEO_NETWORK_CLIENT_CONSTANTS_H

#include "Types.h"

const size_t kTrustLineAmountBytesCount = 32;
const size_t kTrustLineBalanceBytesCount = 32;
const size_t kTrustLineBalanceSerializeBytesCount = 33;

static const constexpr char kCommandsSeparator = '\n';
static const constexpr char kTokensSeparator = '\t';
static const constexpr char kAddressTypeSeparator = '-';
static const constexpr char kAddressSeparator = ':';

static const string kAmountLimit = std::numeric_limits<TrustLineAmount>::max().convert_to<std::string>();


#endif //GEO_NETWORK_CLIENT_CONSTANTS_H
