#ifndef GEO_NETWORK_CLIENT_MULTIPRECISIONUTILS_H
#define GEO_NETWORK_CLIENT_MULTIPRECISIONUTILS_H

#include "../Types.h"

#include <boost/endian/arithmetic.hpp>
#include <vector>

using namespace std;

inline TrustLineAmount absoluteBalanceAmount(
    const TrustLineBalance &balance)
{
    if (balance > TrustLineBalance(0)) {
        return TrustLineAmount(balance);
    } else {
        return TrustLineAmount(-1 * balance);
    }
}

inline vector<byte> trustLineAmountToBytes(
    const TrustLineAmount &amount) {

    vector<byte> rawExportedBytesBuffer, resultBytesBuffer;

    // Exporting bytes of the "amount".
    rawExportedBytesBuffer.reserve(kTrustLineAmountBytesCount);
    export_bits(amount, back_inserter(rawExportedBytesBuffer), 8);

    // Prepending received bytes by zeroes until 32 bytes would be used.
    resultBytesBuffer.reserve(kTrustLineAmountBytesCount);

    size_t unusedBytesCount = kTrustLineAmountBytesCount - rawExportedBytesBuffer.size();
    for (size_t i = 0; i < unusedBytesCount; ++i) {
        resultBytesBuffer.push_back(0);
    }

    size_t usedBytesCount = rawExportedBytesBuffer.size();
    for (size_t i = 0; i < usedBytesCount; ++i) {
        resultBytesBuffer.push_back(
            // Casting each byte to big endian makes the deserializer independent
            // from current machine architecture, and, as result - platfrom portable.
            boost::endian::native_to_big(
                rawExportedBytesBuffer[i]));
    }

    return resultBytesBuffer;
}

inline vector<byte> trustLineBalanceToBytes(
    const TrustLineBalance &balance) {

    vector<byte> rawExportedBytesBuffer, resultBytesBuffer;
    // Exporting bytes of the "balance".
    rawExportedBytesBuffer.reserve(kTrustLineAmountBytesCount);
    export_bits(balance, back_inserter(rawExportedBytesBuffer), 8);

    // Prepending received bytes by zeroes until 32 bytes would be used.
    resultBytesBuffer.reserve(kTrustLineAmountBytesCount + 1);

    size_t unusedBytesCount = kTrustLineAmountBytesCount - rawExportedBytesBuffer.size();
    for (size_t i = 0; i < unusedBytesCount; ++i) {
        resultBytesBuffer.push_back(0);
    }

    size_t usedBytesCount = rawExportedBytesBuffer.size();
    for (size_t i = 0; i < usedBytesCount; ++i) {
        resultBytesBuffer.push_back(
            // Casting each byte to big endian makes the deserializer independent
            // from current machine architecture, and, as result - platfrom portable.
            boost::endian::native_to_big(
                rawExportedBytesBuffer[i]));
    }

    // Process sign
    resultBytesBuffer.insert(
        resultBytesBuffer.begin(),
        boost::endian::native_to_big(
            byte(balance < 0)));

    return resultBytesBuffer;
}

inline TrustLineAmount bytesToTrustLineAmount(
    const vector<byte> &amountBytes) {

    vector<byte> internalBytesBuffer;
    internalBytesBuffer.reserve(kTrustLineAmountBytesCount);

    for (size_t i=0; i<kTrustLineAmountBytesCount; ++i) {
        internalBytesBuffer.push_back(
            boost::endian::big_to_native(
                amountBytes[i]));
    }

    TrustLineAmount amount;
    import_bits(
        amount,
        internalBytesBuffer.begin(),
        internalBytesBuffer.end());

    return amount;
}

inline TrustLineBalance bytesToTrustLineBalance(
    const vector<byte> &balanceBytes) {

    vector<byte> internalBytesBuffer;
    internalBytesBuffer.reserve(kTrustLineAmountBytesCount);

    // Note: sign byte must be skipped, so the cycle is starting from 1.
    for (size_t i=1; i<kTrustLineAmountBytesCount+1; ++i) {
        internalBytesBuffer.push_back(
            boost::endian::big_to_native(
                balanceBytes[i]));
    }

    TrustLineBalance balance;
    import_bits(
        balance,
        internalBytesBuffer.begin(),
        internalBytesBuffer.end());

    // Sign must be processed only in case if balance != 0.
    // By default, after deserialization, balance is always positive,
    // so it must be only checked for > 0, and not != 0.
    if (balance > 0) {
        byte sign = boost::endian::big_to_native(balanceBytes[0]);
        if (sign != 0) {
            balance = balance * -1;
        }
    }

    return balance;
}


#endif //GEO_NETWORK_CLIENT_MULTIPRECISIONUTILS_H
