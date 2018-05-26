#include "CryptoKeyRecord.h"

CryptoKeyRecord::CryptoKeyRecord(
    BytesShared publicKey,
    size_t publicKeySize,
    BytesShared privateKey,
    size_t privateKeySize, uint32_t number):
    mPublicKey(publicKey),
    mPublicKeySize(publicKeySize),
    mPrivateKey(privateKey),
    mPrivateKeySize(privateKeySize)
{}

const pair<BytesShared, size_t> CryptoKeyRecord::publicKey() const
{
    return make_pair(
        mPublicKey,
        mPublicKeySize);
}

const pair<BytesShared, size_t> CryptoKeyRecord::privateKey() const
{
    return make_pair(
        mPrivateKey,
        mPrivateKeySize);
}

const uint32_t CryptoKeyRecord::number() const
{
    return mNumber;
}

uint32_t CryptoKeyRecord::publicKeyHash() const
{
    return 0;
}