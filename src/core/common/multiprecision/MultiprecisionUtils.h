#ifndef GEO_NETWORK_CLIENT_MULTIPRECISIONUTILS_H
#define GEO_NETWORK_CLIENT_MULTIPRECISIONUTILS_H

#include "../Types.h"

#include <vector>

using namespace std;

inline vector<byte> trustLineAmountToBytes(
    const TrustLineAmount &amount) {

    vector<byte> buffer;

    export_bits(
      amount,
      back_inserter(buffer),
      8
    );

    size_t unusedBufferPlace = kTrustLineAmountBytesCount - buffer.size();
    for (size_t i = 0; i < unusedBufferPlace; ++i) {
        buffer.insert(buffer.begin(), 0);
    }

    return buffer;
}

inline TrustLineAmount bytesToTrustLineAmount(
    const vector<byte> &amountBytes) {

    TrustLineAmount amount;

    import_bits(
        amount,
        amountBytes.begin(),
        amountBytes.end()
    );

    return amount;
}

inline vector<byte> trustLineBalanceToBytes(
    TrustLineBalance balance) {

    vector<byte> buffer;

    bool isSignNegative = false;
    if (balance.sign() == -1) {
        isSignNegative = true;
        balance = balance * -1;
    }

    export_bits(
        balance,
        back_inserter(buffer),
        8
    );

    size_t unusedBufferPlace = kTrustLineAmountBytesCount - buffer.size();
    for (size_t i = 0; i < unusedBufferPlace; ++i) {
        buffer.insert(buffer.begin(), 0);
    }

    if (isSignNegative) {
        buffer.insert(buffer.begin(), 1);
        balance = balance * -1;

    } else {
        buffer.insert(buffer.begin(), 0);
    }

    return buffer;
}

inline TrustLineBalance bytesToTrustLineBalance(
    const vector<byte> balanceBytes) {

    TrustLineBalance balance;

    byte sign = balanceBytes.at(0);
    import_bits(
        balance,
        balanceBytes.begin() + 1,
        balanceBytes.end()
    );

    if (sign == 1) {
        balance = balance * -1;
    }

    return balance;
}

inline TrustLineAmount absoluteBalanceAmount(
    const TrustLineBalance &balance)
{
    if (balance > TrustLineBalance(0)) {
        return TrustLineAmount(balance);
    } else {
        return TrustLineAmount(-1 * balance);
    }
}

#endif //GEO_NETWORK_CLIENT_MULTIPRECISIONUTILS_H
