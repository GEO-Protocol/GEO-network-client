#include "lamportscheme.h"


namespace crypto {
namespace lamport {


Signature::Signature(
    byte *data,
    size_t dataSize,
    PrivateKey *pKey) {

    if (pKey->mIsCropped) {
        // todo: throw runtime error.
        exit(1);
    }

    mData = static_cast<byte*>(malloc(kSize));
    if (mData == nullptr) {
        // todo: throw memory error
        return;
    }

    const auto hashSize = PrivateKey::kRandomNumberSize;

    byte messageHash[hashSize];
    crypto_generichash(messageHash, hashSize, data, dataSize, nullptr, 0);

    {
        auto pKeyGuard = pKey->mData.unlockAndInitGuard();
        collectSignature(pKeyGuard.address(), mData, messageHash);
    }

    {
        // Cropping the private key.
        // This is needed to prevent it reuse.
        pKey->mIsCropped = true;
    }
}

Signature::Signature(
    byte *data)
{
    mData = static_cast<byte*>(
        malloc(
            signatureSize()));
    memcpy(
        mData,
        data,
        signatureSize());
}

Signature::~Signature(){
    if (mData != nullptr) {
        free(mData);
        mData = nullptr;
    }
}

const size_t Signature::signatureSize()
{
    // signature has 8KB
    return 8 * 1024;
}

const byte* Signature::data() const
{
    return mData;
}

bool Signature::check(
    byte *data,
    size_t dataSize,
    PublicKey::Shared pubKey)
    noexcept {

    if (mData == nullptr || dataSize == 0) {
        return false;
    }

    auto hashedSignature = static_cast<byte*>(malloc(kSize));
    if (hashedSignature == nullptr) {
        return false;
    }

    auto pubKeySignature = static_cast<byte*>(malloc(kSize));
    if (pubKeySignature == nullptr) {
        free(hashedSignature);
        return false;
    }

    // Collecting pub key signature.
    const auto hashSize = PublicKey::kRandomNumberSize;
    byte messageHash[hashSize];
    crypto_generichash(messageHash, hashSize, data, dataSize, nullptr, 0);
    collectSignature(pubKey->mData, pubKeySignature, messageHash);

    // Collecting hashed signature.
    auto originalSignatureOffset = mData;
    auto hashedSignatureOffset = hashedSignature;

    for (size_t i=0; i<PublicKey::kRandomNumbersCount / 2; ++i) {
        crypto_generichash(hashedSignatureOffset, hashSize, originalSignatureOffset, hashSize, nullptr, 0);
        originalSignatureOffset += PublicKey::kRandomNumberSize;
        hashedSignatureOffset += PublicKey::kRandomNumberSize;
    }

    // Comparing results.
    auto result = memcmp(pubKeySignature, hashedSignature, kSize);
    free(hashedSignature);
    free(pubKeySignature);

    return !result;
}

void Signature::collectSignature(
    byte *key,
    byte *signature,
    byte *messageHash)
    noexcept {

    const auto bitsInByte = 8;
    auto signatureOffset = signature;
    auto numbersPairOffset = key;

    for (size_t i=0; i<PrivateKey::kRandomNumberSize; ++i) {
        std::bitset<bitsInByte> byteOfMessageHash(messageHash[i]);

        for (size_t b=0; b<bitsInByte; ++b) {
            auto source = numbersPairOffset+PrivateKey::kRandomNumberSize;
            if (byteOfMessageHash.test(b)) {
                source = numbersPairOffset;
            }

            memcpy(signatureOffset, source, PrivateKey::kRandomNumberSize);
            numbersPairOffset += PrivateKey::kRandomNumberSize * 2;
            signatureOffset += PrivateKey::kRandomNumberSize;
        }
    }
}


}
}

