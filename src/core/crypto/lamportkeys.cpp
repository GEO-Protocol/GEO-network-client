#include "lamportkeys.h"


namespace crypto {
namespace lamport {

const size_t BaseKey::keySize()
{
    // public and private keys has 16KB
    return 16 * 1024;
}


PrivateKey::PrivateKey():
    mData(memory::SecureSegment(kRandomNumbersCount * kRandomNumberSize)),
    mIsCropped(false) {

    auto guard = mData.unlockAndInitGuard();

    auto offset = static_cast<byte*>(guard.address());
    for (size_t i=0; i<kRandomNumbersCount; ++i) {
        randombytes_buf(offset, kRandomNumberSize);
        offset += kRandomNumberSize;
    }
}


PublicKey::Shared PrivateKey::derivePublicKey() {

    auto guard = mData.unlockAndInitGuard();
    auto generatedKey = make_shared<PublicKey>();

    // Numbers buffers memory allocation.
    generatedKey->mData = static_cast<byte*>(malloc(kRandomNumbersCount * kRandomNumberSize));
    if (generatedKey->mData == nullptr) {
        return nullptr;
    }

    // Numbers buffers initialisation via hashing private key numbers.
    auto source = static_cast<unsigned char*>(guard.address());
    auto destination = static_cast<unsigned char*>(generatedKey->mData);

    for (size_t i=0; i<kRandomNumbersCount; ++i) {
        // Warn: NULL is required! nullptr causes the internal lib error.
        crypto_generichash(destination, kRandomNumberSize, source, kRandomNumberSize, nullptr, 0);
        source += kRandomNumberSize;
        destination += kRandomNumberSize;
    }

    return generatedKey;
}

const byte* PrivateKey::data() const
{
    return mData.address();
}

PublicKey::PublicKey(
    byte *data)
{
    mData = static_cast<byte*>(
        malloc(
            keySize()));
    memcpy(
        mData,
        data,
        keySize());
}

PublicKey::~PublicKey()
    noexcept
{
    if (mData != nullptr) {
        free(mData);
        mData = nullptr;
    }
}

const byte* PublicKey::data() const
{
    return mData;
}

const uint32_t PublicKey::hash() const
{
    // todo return hash and change return type
    return 0;
}

const uint64_t PublicKey::crc() const
{
    // todo return crc and change return type
    return 0;
}

}
}

