#ifndef GEO_NETWORK_CLIENT_CRYPTOKEY_H
#define GEO_NETWORK_CLIENT_CRYPTOKEY_H

#include "../common/Types.h"
#include "../common/memory/MemoryUtils.h"
#include "../common/multiprecision/MultiprecisionUtils.h"

#include <boost/crc.hpp>

class CryptoKey {

public:
    CryptoKey();

    CryptoKey(
        byte* key,
        size_t keySize);

    ~CryptoKey();

    void deserialize(
        size_t keySize,
        unsigned char* buffer);

    byte* key() const;

    size_t keySize() const;

    uint32_t hash() const;

    uint64_t crc() const;

    pair<BytesShared, size_t> signData(
        BytesShared data,
        size_t dataBytesCount) const;

    // returned if data correct and row data without sign
    tuple<bool, BytesShared, size_t> checkData(
        BytesShared signedData,
        size_t signedDataSize);

    static size_t serializedKeySize();

private:
    byte* mKey;
    size_t mKeySize;
};


#endif //GEO_NETWORK_CLIENT_CRYPTOKEY_H
