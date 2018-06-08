#ifndef GEO_NETWORK_CLIENT_KEYCHAIN_H
#define GEO_NETWORK_CLIENT_KEYCHAIN_H

#include "../common/Types.h"
#include "CryptoKey.h"

#include "../io/storage/IOTransaction.h"
#include "../logger/Logger.h"

#include <tuple>

class KeyChain {

public:
    KeyChain(
        const TrustLineID trustLineID,
        Logger &logger);

    static KeyChain makeKeyChain(
        const TrustLineID trustLineID,
        Logger &logger);

    void initGeneration(
        uint32_t keysCnt,
        IOTransaction::Shared ioTransaction);

    vector<pair<uint32_t, CryptoKey>> allAvailablePublicKeys(
        IOTransaction::Shared ioTransaction);

    void saveContractorPublicKey(
        IOTransaction::Shared ioTransaction,
        uint32_t number,
        const CryptoKey &publicKey);

    bool isAllKeysReady(
        IOTransaction::Shared ioTransaction,
        uint32_t keysCnt);

    // returned signed data (bytes and size) and key number
    tuple<BytesShared, size_t, uint32_t> signData(
        IOTransaction::Shared ioTransaction,
        BytesShared data,
        size_t dataBytesCount);

    // returned if data correct and row data without sign
    tuple<bool, BytesShared, size_t> checkSignedData(
        IOTransaction::Shared ioTransaction,
        BytesShared data,
        size_t dataBytesCount,
        uint32_t keyNumber);

    // return public key hash
    uint32_t generateAndSaveKeyPairForPaymentTransaction(
        const TransactionUUID &transactionUUID);

    CryptoKey& paymentPublicKey(
        uint32_t publicKeyHash);

protected:
    string logHeader() const;

    LoggerStream warning() const;

    LoggerStream error() const;

    LoggerStream info() const;

    LoggerStream debug() const;

private:
    void generateAndSaveKeyPair(
        uint32_t keyNumber,
        IOTransaction::Shared ioTransaction);

private:
    TrustLineID mTrustLineID;
    Logger &mLogger;
};


#endif //GEO_NETWORK_CLIENT_KEYCHAIN_H
