#ifndef GEO_NETWORK_CLIENT_PROVIDERMSGENCRYPTOR_H
#define GEO_NETWORK_CLIENT_PROVIDERMSGENCRYPTOR_H

#include <sodium.h>
#include "../common/memory/MemoryUtils.h"
#include "../common/time/TimeUtils.h"

class ProviderMsgEncryptor {

public:
    typedef pair<BytesShared, size_t> Buffer;
    struct PublicKey {
        typedef std::shared_ptr<PublicKey> Shared;
        static const size_t kBytesSize = crypto_aead_aes256gcm_KEYBYTES;
        PublicKey() {}
        explicit PublicKey(const string &str);
        byte key[kBytesSize];
    };

public:
    explicit ProviderMsgEncryptor(
        const PublicKey::Shared &publicKey);

    Buffer encrypt(
        byte* bytesForEncryption,
        size_t size);

protected:
    PublicKey::Shared mPublicKey = nullptr;
};


#endif //GEO_NETWORK_CLIENT_PROVIDERMSGENCRYPTOR_H
