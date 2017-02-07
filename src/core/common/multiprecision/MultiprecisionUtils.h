#ifndef GEO_NETWORK_CLIENT_MULTIPRECISIONUTILS_H
#define GEO_NETWORK_CLIENT_MULTIPRECISIONUTILS_H

#include "../Types.h"

#include <vector>

using namespace std;

inline vector<byte> trustLineAmountToBytes(
    TrustLineAmount &amount) {

    vector<byte> buffer;
    buffer.reserve(kTrustLineAmountBytesCount);

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
    vector<byte> amountBytes) {

    TrustLineAmount amount;

    vector<byte> amountNotZeroBytes;
    amountNotZeroBytes.reserve(kTrustLineAmountBytesCount);

    for (auto &item : amountBytes) {
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

#endif //GEO_NETWORK_CLIENT_MULTIPRECISIONUTILS_H
