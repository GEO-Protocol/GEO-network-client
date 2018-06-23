#include "keychain.h"


namespace crypto {


Encryptor::Encryptor(
    memory::SecureSegment &key)
    noexcept:

    mKey(key)
{}


pair<BytesShared, size_t> Encryptor::encrypt(
    byte *data,
    size_t len)
{
    mKey.unlockAndInitGuard();
    // ...
}

pair<BytesShared, size_t> Encryptor::decrypt(
    byte *data,
    size_t len)
{
    mKey.unlockAndInitGuard();
    // ...
}


Keystore::Keystore(
    //todo memory::SecureSegment &memoryKey,
    Logger &logger)
    noexcept:

    //todo mEncryptor(Encryptor(memoryKey)),
    mLogger(logger)
{}

int Keystore::init()
{
    return sodium_init();
}

TrustLineKeychain Keystore::keychain(
    const TrustLineID trustLineID) const
{
    return TrustLineKeychain(
        trustLineID,
        // todo mEncryptor,
        mLogger);
}

lamport::PublicKey::Shared Keystore::generateAndSaveKeyPairForPaymentTransaction(
    IOTransaction::Shared ioTransaction,
    const TransactionUUID &transactionUUID,
    const NodeUUID &nodeUUID)
{
    lamport::PrivateKey pKey;
    auto pubKey = pKey.derivePublicKey();
    ioTransaction->paymentKeysHandler()->saveOwnKey(
        transactionUUID,
        nodeUUID,
        pubKey,
        &pKey);
    return pubKey;
}

lamport::Signature::Shared Keystore::signPaymentTransaction(
    IOTransaction::Shared ioTransaction,
    const TransactionUUID &transactionUUID,
    BytesShared dataForSign,
    size_t dataForSignBytesCount)
{
    auto privateKey = ioTransaction->paymentKeysHandler()->getOwnPrivateKey(
        transactionUUID);
    return make_shared<Signature>(
        dataForSign.get(),
        dataForSignBytesCount,
        privateKey);
}

LoggerStream Keystore::info() const
{
    return mLogger.info(logHeader());
}

LoggerStream Keystore::debug() const
{
    return mLogger.debug(logHeader());
}

LoggerStream Keystore::warning() const
{
    return mLogger.warning(logHeader());
}

const string Keystore::logHeader() const
{
    stringstream s;
    s << "[Keystore] ";
    return s.str();
}

TrustLineKeychain::TrustLineKeychain(
    const TrustLineID trustLineID,
    //todo Encryptor encryptor,
    Logger &logger)
    noexcept:

    mTrustLineID(trustLineID),
    //todo mEncryptor(encryptor),
    mLogger(logger)
{}

void TrustLineKeychain::generateKeyPairsSet(
    IOTransaction::Shared ioTransaction,
    KeysCount keyPairsCount)
{
    auto cntFailedAttempts = 0;
    keyNumberGuard(keyPairsCount);
    for (KeyNumber idx = 0; idx < keyPairsCount; idx++) {
        lamport::PrivateKey pKey;
        auto pubKey = pKey.derivePublicKey();
        try {
            ioTransaction->ownKeysHandler()->saveKey(
                mTrustLineID,
                pubKey,
                &pKey,
                idx);
        } catch (IOError &e) {
            warning() << "Can't save keys pair. Details: " << e.what();
            cntFailedAttempts++;
            if (cntFailedAttempts >= 3) {
                throw e;
            }
            idx--;
            continue;
        }
    }
}

pair<lamport::PublicKey::Shared, bool> TrustLineKeychain::publicKey(
    IOTransaction::Shared ioTransaction,
    const KeyNumber number) const
{
    keyNumberGuard(number);

    auto publicKey = ioTransaction->ownKeysHandler()->getPublicKey(
        mTrustLineID,
        number);
    return make_pair(
        publicKey,
        true);
}

void TrustLineKeychain::setContractorPublicKey(
    IOTransaction::Shared ioTransaction,
    KeyNumber number,
    const lamport::PublicKey::Shared key)
{
    keyNumberGuard(number);

    // ...
    // todo: throw ConsistencyError in case if contractor already has key in this position.
    ioTransaction->contractorKeysHandler()->saveKey(
        mTrustLineID,
        key,
        number);
}

bool TrustLineKeychain::areKeysReady(
    IOTransaction::Shared ioTransaction,
    KeysCount count) noexcept
{
    keyNumberGuard(count);

    if (ioTransaction->ownKeysHandler()->availableKeysCnt(mTrustLineID) != count) {
        info() << "There are no all own keys: "
               << ioTransaction->ownKeysHandler()->availableKeysCnt(mTrustLineID);
        return false;
    }
    if (ioTransaction->contractorKeysHandler()->availableKeysCnt(mTrustLineID) != count) {
        info() << "There are no all contractor keys: "
               << ioTransaction->contractorKeysHandler()->availableKeysCnt(mTrustLineID);
        return false;
    }
    return true;
}

pair<lamport::Signature::Shared, KeyNumber> TrustLineKeychain::sign(
    IOTransaction::Shared ioTransaction,
    BytesShared data,
    const std::size_t size)
{
    dataGuard(data, size);

    // todo: throw KeyError if no key is available;

    pair<PrivateKey*, KeyNumber> privateKeyAndNumber;
    try {
        privateKeyAndNumber = ioTransaction->ownKeysHandler()->nextAvailableKey(
            mTrustLineID);
    } catch (NotFoundError &e) {
        warning() << "Can't get available private key for TL " << mTrustLineID;
        throw e;
    }

    auto signature = make_shared<lamport::Signature>(
        data.get(),
        size,
        privateKeyAndNumber.first);
    // todo: read PKey
    // todo: decrypt it.
    // todo: read sign
    // todo: !! store cutted private key back and mark it as used.

    return make_pair(
        signature,
        privateKeyAndNumber.second);
}

bool TrustLineKeychain::checkSign(
    IOTransaction::Shared ioTransaction,
    BytesShared data,
    const size_t size,
    const lamport::Signature::Shared signature,
    const KeyNumber keyNumber)
{
    dataGuard(data, size);
    keyNumberGuard(keyNumber);

    try {
        auto contractorPublicKey = ioTransaction->contractorKeysHandler()->keyByNumber(
            mTrustLineID,
            keyNumber);
        return signature->check(
            data.get(),
            size,
            contractorPublicKey);
    } catch (NotFoundError &e) {
        warning() << "There are no data for TL " << mTrustLineID << " and keyNumber " << keyNumber;
        return false;
    } catch (IOError &e) {
        warning() << "Can't get contractor public key. Details: " << e.what();
        return false;
    }
}

void TrustLineKeychain::saveAudit(
    IOTransaction::Shared ioTransaction,
    const AuditNumber auditNumber,
    const KeyNumber ownKeyNumber,
    const lamport::Signature::Shared ownSignature,
    const KeyNumber contractorKeyNumber,
    const lamport::Signature::Shared contractorSignature,
    const TrustLineAmount &incomingAmount,
    const TrustLineAmount &outgoingAmount,
    const TrustLineBalance &balance)
{
    auto ownKeyHash = ioTransaction->ownKeysHandler()->getPublicKeyHash(
        mTrustLineID,
        ownKeyNumber);
    auto contractorKeyHash = ioTransaction->contractorKeysHandler()->keyHashByNumber(
        mTrustLineID,
        contractorKeyNumber);

    ioTransaction->auditHandler()->saveAudit(
        auditNumber,
        mTrustLineID,
        ownKeyHash,
        ownSignature,
        contractorKeyHash,
        contractorSignature,
        incomingAmount,
        outgoingAmount,
        balance);
}

void TrustLineKeychain::keyNumberGuard(
    const KeyNumber &number) const
{
    if (number == 0 || number > kMaxKeysSetSize) {
        // todo: throw ValueError;
    }
}

void TrustLineKeychain::dataGuard(
    const BytesShared data,
    const size_t size) const
{

    if (data == nullptr) {
        // todo: throw ValueError;
    }

    if (size == 0) {
        // todo: throw ValueError;
    }
}

LoggerStream TrustLineKeychain::info() const
{
    return mLogger.info(logHeader());
}

LoggerStream TrustLineKeychain::debug() const
{
    return mLogger.debug(logHeader());
}

LoggerStream TrustLineKeychain::warning() const
{
    return mLogger.warning(logHeader());
}

const string TrustLineKeychain::logHeader() const
{
    stringstream s;
    s << "[TrustLineKeychain: " << mTrustLineID << "] ";
    return s.str();
}


}
