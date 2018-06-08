#include "KeyChain.h"

KeyChain::KeyChain(
    const TrustLineID trustLineID,
    Logger &logger):
    mTrustLineID(trustLineID),
    mLogger(logger)
{
    srand(234523);
}

KeyChain KeyChain::makeKeyChain(
    const TrustLineID trustLineID,
    Logger &logger)
{
    return KeyChain(
        trustLineID,
        logger);
}

void KeyChain::initGeneration(
    uint32_t keysCnt,
    IOTransaction::Shared ioTransaction)
{
    for (uint32_t idx = 0; idx < keysCnt; idx++) {
        generateAndSaveKeyPair(
            idx,
            ioTransaction);
    }
}

void KeyChain::generateAndSaveKeyPair(
    uint32_t keyNumber,
    IOTransaction::Shared ioTransaction)
{
    byte privateKeyValue = (byte)(rand() % 255);
    auto privateKey = CryptoKey(&privateKeyValue, 1);
    byte publicKeyValue = (byte)255 - privateKeyValue;
    auto publicKey = CryptoKey(&publicKeyValue, 1);
    info() << "generate private key: " << (int)*privateKey.key() << " public key: " << (int)*publicKey.key();
    ioTransaction->ownKeysHandler()->saveKey(
        mTrustLineID,
        publicKey,
        privateKey,
        keyNumber);
}

vector<pair<uint32_t, CryptoKey>> KeyChain::allAvailablePublicKeys(
    IOTransaction::Shared ioTransaction)
{
    info() << "allAvailablePublicKeys";
    return ioTransaction->ownKeysHandler()->allAvailablePublicKeys(
        mTrustLineID);
}

void KeyChain::saveContractorPublicKey(
    IOTransaction::Shared ioTransaction,
    uint32_t keyNumber,
    const CryptoKey &publicKey)
{
    ioTransaction->contractorKeysHandler()->saveKey(
        mTrustLineID,
        publicKey,
        keyNumber);
}

bool KeyChain::isAllKeysReady(
    IOTransaction::Shared ioTransaction,
    uint32_t keysCnt)
{
    if (ioTransaction->ownKeysHandler()->availableKeysCnt(mTrustLineID) != keysCnt) {
        info() << "There are no all own keys: "
               << ioTransaction->ownKeysHandler()->availableKeysCnt(mTrustLineID);
        return false;
    }
    if (ioTransaction->contractorKeysHandler()->availableKeysCnt(mTrustLineID) != keysCnt) {
        info() << "There are no all contractor keys: "
               << ioTransaction->contractorKeysHandler()->availableKeysCnt(mTrustLineID);
        return false;
    }
    return true;
}

tuple<BytesShared, size_t, uint32_t> KeyChain::signData(
    IOTransaction::Shared ioTransaction,
    BytesShared data,
    size_t dataBytesCount)
{
    auto numberAndPrivateKey = ioTransaction->ownKeysHandler()->nextAvailableKey(mTrustLineID);
    info() << "our private key value: " << (int)*numberAndPrivateKey.second.key();
    auto signDataAndSize = numberAndPrivateKey.second.signData(
        data,
        dataBytesCount);
    // todo marked key as used
    return make_tuple(
        signDataAndSize.first,
        signDataAndSize.second,
        numberAndPrivateKey.first);
}

tuple<bool, BytesShared, size_t> KeyChain::checkSignedData(
    IOTransaction::Shared ioTransaction,
    BytesShared data,
    size_t dataBytesCount,
    uint32_t keyNumber)
{
    info() << "checkSignedData";
    // todo check if key is present and valid
    auto contractorPublicKey = ioTransaction->contractorKeysHandler()->keyByNumber(keyNumber);
    info() << "contractor public key value: " << (int)*contractorPublicKey.key();
    return contractorPublicKey.checkData(
        data,
        dataBytesCount);
}

uint32_t KeyChain::generateAndSaveKeyPairForPaymentTransaction(
    const TransactionUUID &transactionUUID)
{
    // todo : save key to storage
    return 0;
}

CryptoKey& KeyChain::paymentPublicKey(
    uint32_t publicKeyHash)
{
    CryptoKey result;
    return result;
}

string KeyChain::logHeader() const
{
    return "[KeyChain " + to_string(mTrustLineID) + "]";
}

LoggerStream KeyChain::error() const
{
    return mLogger.error(logHeader());
}

LoggerStream KeyChain::warning() const
{
    return mLogger.warning(logHeader());
}

LoggerStream KeyChain::info() const
{
    return mLogger.info(logHeader());
}

LoggerStream KeyChain::debug() const
{
    return mLogger.debug(logHeader());
}
