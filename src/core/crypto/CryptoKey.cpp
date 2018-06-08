#include "CryptoKey.h"

CryptoKey::CryptoKey(
    byte *key,
    size_t keySize)
{
    mKey = (byte*)malloc(keySize);
    memcpy(
        mKey,
        key,
        keySize);
    mKeySize = keySize;
}

CryptoKey::CryptoKey():
    mKeySize(0),
    mKey(nullptr)
{}

CryptoKey::~CryptoKey()
{
    //free(mKey);
}

void CryptoKey::deserialize(
    size_t keySize,
    unsigned char* buffer)
{
    mKey = (byte*)malloc(keySize);
    memcpy(
        mKey,
        buffer,
        keySize);
    mKeySize = keySize;
}

byte* CryptoKey::key() const
{
    return mKey;
}

size_t CryptoKey::keySize() const
{
    return mKeySize;
}

uint32_t CryptoKey::hash() const
{
    return *mKey;
}

uint64_t CryptoKey::crc() const
{
//    boost::crc_32_type result;
//    result.process_bytes(mKey, mKeySize);
//    return result.checksum();
    return (*mKey) * 100;
}

pair<BytesShared, size_t> CryptoKey::signData(
    BytesShared data,
    size_t dataBytesCount) const
{
    size_t resultBytesCount = mKeySize + dataBytesCount;
    BytesShared result = tryMalloc(resultBytesCount);
    memcpy(
        result.get(),
        mKey,
        mKeySize);
    memcpy(
        result.get() + mKeySize,
        data.get(),
        dataBytesCount);
    return make_pair(
        result,
        resultBytesCount);
}

tuple<bool, BytesShared, size_t> CryptoKey::checkData(
    BytesShared signedData,
    size_t signedDataSize)
{
    byte privateKeyValue;
    memcpy(
        &privateKeyValue,
        signedData.get(),
        mKeySize);

    if (privateKeyValue + (byte)*mKey != 255) {
        return make_tuple(false, signedData, signedDataSize);
    }

    size_t rawDataSize = signedDataSize - mKeySize;
    BytesShared rawData = tryMalloc(rawDataSize);
    memcpy(
        rawData.get(),
        signedData.get() + mKeySize,
        rawDataSize);
    return make_tuple(true, rawData, rawDataSize);
}

size_t CryptoKey::serializedKeySize()
{
    // todo add default key size
    return sizeof(size_t) + 4;
}