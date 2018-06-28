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
    mIsCropped(false)
{
    auto guard = mData.unlockAndInitGuard();

    auto offset = static_cast<byte*>(guard.address());
    for (size_t i=0; i<kRandomNumbersCount; ++i) {
        randombytes_buf(offset, kRandomNumberSize);
        offset += kRandomNumberSize;
    }
}

PrivateKey::PrivateKey(
    byte *data) :
    mData(memory::SecureSegment(keySize())),
    mIsCropped(false)
{
    auto guard = mData.unlockAndInitGuard();
    auto offset = static_cast<byte*>(guard.address());
    memcpy(
        offset,
        data,
        keySize());
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
    auto source = static_cast<byte*>(guard.address());
    auto destination = static_cast<byte*>(generatedKey->mData);

    for (size_t i=0; i<kRandomNumbersCount; ++i) {
        crypto_generichash(destination, kRandomNumberSize, source, kRandomNumberSize, nullptr, 0);
        source += kRandomNumberSize;
        destination += kRandomNumberSize;
    }

    return generatedKey;
}

const memory::SecureSegment* PrivateKey::data() const
{
    return &mData;
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

const KeyHash::Shared PublicKey::hash() const
{
    // todo build correct hash
    return make_shared<KeyHash>(mData);
}

KeyHash::KeyHash(
    byte* buffer)
{
    memcpy(
        mData,
        buffer,
        kBytesSize);
}

const byte* KeyHash::data() const
{
    return mData;
}

bool operator== (const KeyHash &kh1, const KeyHash &kh2)
{
#ifdef BOOST_BIG_ENDIAN
    //todo
#endif

#ifdef BOOST_LITTLE_ENDIAN
    for (int i = KeyHash::kBytesSize - 1; i >= 0; --i){
        if (kh1.mData[i] != kh2.mData[i])
            return false;
    }
    return true;
#endif
}

bool operator!= (const KeyHash &kh1, const KeyHash &kh2)
{
#ifdef BOOST_BIG_ENDIAN
        //todo
#endif

#ifdef BOOST_LITTLE_ENDIAN
    for (int i = KeyHash::kBytesSize - 1; i >= 0; --i){
        if (kh1.mData[i] != kh2.mData[i])
            return true;
    }
    return false;
#endif
}

}
}

