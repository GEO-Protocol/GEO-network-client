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
        buffer.push_back(0);
    }

    return buffer;
}

inline TrustLineAmount bytesToTrustLineAmount(
    const vector<byte> &amountBytes) {

    TrustLineAmount amount;

    vector<byte> amountNotZeroBytes;
    amountNotZeroBytes.reserve(kTrustLineAmountBytesCount);

    for (const auto &item : amountBytes) {
        if (item != 0) {
            amountNotZeroBytes.push_back(item);
        }
    }

    if (amountNotZeroBytes.size() > 0) {
        import_bits(
            amount,
            amountNotZeroBytes.begin(),
            amountNotZeroBytes.end()
        );

    } else {
        import_bits(
            amount,
            amountBytes.begin(),
            amountBytes.end()
        );
    }

    return amount;
}

inline vector<byte> trustLineBalanceToBytes(
    TrustLineBalance balance) {

    vector<byte> buffer;

    bool isSignNegative = false;
    if (balance.sign() == -1) {
        balance = balance * -1;
        isSignNegative = true;
    }

    export_bits(
        balance,
        back_inserter(buffer),
        8
    );

    size_t unusedBufferPlace = kTrustLineBalanceBytesCount - buffer.size();
    for (size_t i = 0; i < unusedBufferPlace; ++i) {
        buffer.push_back(0);
    }

    if (isSignNegative) {
        buffer.push_back(1);
        balance = balance * -1;

    } else {
        buffer.push_back(0);
    }

    return buffer;
}

inline TrustLineBalance bytesToTrustLineBalance(
    const vector<byte> balanceBytes) {

    TrustLineBalance balance;

    vector<byte> notZeroBytesVector;
    notZeroBytesVector.reserve(kTrustLineBalanceBytesCount);

    byte sign = balanceBytes.at(balanceBytes.size() - 1);

    for (size_t byteIndex = 0; byteIndex < balanceBytes.size(); ++ byteIndex) {
        if (byteIndex != balanceBytes.size() - 1) {
            byte byteValue = balanceBytes.at(byteIndex);
            if (byteValue != 0) {
                notZeroBytesVector.push_back(byteValue);
            }
        }
    }

    if (notZeroBytesVector.size() > 0) {
        import_bits(
            balance,
            notZeroBytesVector.begin(),
            notZeroBytesVector.end()
        );

    } else {
        import_bits(
            balance,
            balanceBytes.begin(),
            balanceBytes.end()
        );
    }

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
