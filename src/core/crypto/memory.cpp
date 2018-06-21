#include "memory.h"


namespace crypto {
namespace memory {


SecureSegment::SecureSegment(
    size_t bytesCount):
    mSize(bytesCount) {

    mAddress = static_cast<byte*>(sodium_malloc(bytesCount));
    if (mAddress == nullptr) {
        // todo: throw memory error on codebase merge;
        exit(-1);
    }

    auto locked = (sodium_mprotect_noaccess(mAddress) == 0);
    if (not locked) {
        wipeAndFree();

        // todo: throw memory error on codebase merge;
        exit(-1);
    }
}

SecureSegment::~SecureSegment()
    noexcept {

    wipeAndFree();
}

SecureSegmentGuard SecureSegment::unlockAndInitGuard()
    const
    noexcept {

    return SecureSegmentGuard(const_cast<SecureSegment&>(*this));
}

byte *SecureSegment::address()
    const
    noexcept {

    return mAddress;
}

void SecureSegment::wipeAndFree()
    noexcept {

    if (mAddress != nullptr) {
        sodium_free(mAddress);
        mAddress = nullptr;
        mSize = 0;
    }
}


SecureSegmentGuard::SecureSegmentGuard(
    SecureSegment &segment)
    noexcept:

    mSegment(segment){

   sodium_mprotect_readwrite(mSegment.address());
}

SecureSegmentGuard::~SecureSegmentGuard()
    noexcept {

   sodium_mprotect_noaccess(mSegment.address());
}

byte *SecureSegmentGuard::address() const noexcept {
    return mSegment.address();
}


}
}
