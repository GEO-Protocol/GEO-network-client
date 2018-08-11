#ifndef KEYCHAIN_H
#define KEYCHAIN_H

#include "memory.h"
#include "lamportscheme.h"
#include "../logger/Logger.h"
#include "../transactions/transactions/base/TransactionUUID.h"
#include "../io/storage/IOTransaction.h"
#include "../io/storage/record/audit/AuditRecord.h"
#include "../io/storage/record/audit/ReceiptRecord.h"

#include <boost/noncopyable.hpp>
#include <vector>
#include <utility>


namespace crypto {
using namespace std;


class Encryptor {
public:
    Encryptor(
        memory::SecureSegment &key)
        noexcept;

    /**
     * @brief
     * Encrypts data with a received key.
     * Doesn't modifies original data.
     *
     * @throws MemoryError on bad allocation attempt.
     */
    pair<BytesShared, size_t> encrypt(
        byte *data,
        size_t len);

    /**
     * @brief
     * Decrypts data with a received key.
     * Doesn't modifies original data.
     *
     * @throws MemoryError on bad allocation attempt.
     */
    pair<BytesShared, size_t> decrypt(
        byte *data,
        size_t len);

private:
    memory::SecureSegment &mKey;
};


/**
 * @brief The Keystore class provides single point of responsibility
 * for the crypto processing of sensitive data.
 * It is expected, that there would be only one instance of this class,
 * that would be available globally for the application sub-systems.
 */
class TrustLineKeychain;
class Keystore:
    public boost::noncopyable {

public:
    Keystore(
        //todo memory::SecureSegment &memoryKey,
        Logger &logger)
        noexcept;

    int init();

    TrustLineKeychain keychain(
        const TrustLineID trustLineID)
        const;

    lamport::PublicKey::Shared generateAndSaveKeyPairForPaymentTransaction(
        IOTransaction::Shared ioTransaction,
        const TransactionUUID &transactionUUID,
        const NodeUUID &nodeUUID);

    lamport::Signature::Shared signPaymentTransaction(
        IOTransaction::Shared ioTransaction,
        const TransactionUUID &transactionUUID,
        BytesShared dataForSign,
        size_t dataForSignBytesCount);

private:
    LoggerStream info() const;

    LoggerStream debug() const;

    LoggerStream warning() const;

    const string logHeader() const;

private:
    Logger &mLogger;
    //todo Encryptor mEncryptor;
};


/**
 * @brief The TrustLineKeychain class provides secure interface
 * for interacting with private and public keys, signing and checking data.
 * It's main purpose is to encapsulate sensitive data processing of the trust line
 * in one place in codebase. It would never provide raw access to the private keys,
 * or any sensitive data. All data signing is done internally.
 */
class TrustLineKeychain{
public:
    static const size_t kDefaultKeysSetSize = 10;     // 16MB of PubKeys, and 16MB of PKeys.
    static const size_t kMaxKeysSetSize = 1024;
    static const size_t kMinKeysSetSize = 2;

public:
    /**
     * @brief
     * Creates keychain for the trust line with id = "trustLineID".
     *
     * @param "encryptor" is used for encrypting/decrypting
     * sensitive data into memory and into the database.
     * Memory is encrypted to prevent sensitive data leaks
     * in case of any dumps and unauthorized memory access attempts.
     * Database data is encrypted to prevent sensitive data leaks
     * through SQLite internal caches, and the OS caches.
     */
    TrustLineKeychain(
        const TrustLineID trustLineID,
        // todo Encryptor encryptor,
        Logger &logger)
        noexcept;

    /**
     * @brief
     * Generates and stores into internal storage "keyPairsCount" of keys.
     *
     * @throws "ValueError" in case if "keyPairsCount" is 0,
     * or is greatest than "kMaxKeysSetSize".
     *
     * @throws "RuntimeError" in case of any internal error.
     * Writes corresponding log record before exception throwing.
     */
    void generateKeyPairsSet(
        IOTransaction::Shared ioTransaction,
        KeysCount keyPairsCount=kDefaultKeysSetSize);

    /**
     * @returns public key with number = "number".
     * Corresponding flag would be set to "true",
     * if key is present and is available for signing the data.
     * Otherwise - corresponding flag would be set to false.
     *
     * @throws "RuntimeError" in case of any internal error.
     * Writes corresponding log record before exception throwing.
     *
     * todo: Mykola, I have replaced allAvailablePublicKeys by this one, to prevent reading of 16MB data.
     * I think we should iteratively read keys one by one
     * to prevent huge memory usage by the nodes in DC on the startup.
     */
    lamport::PublicKey::Shared publicKey(
        IOTransaction::Shared ioTransaction,
        const KeyNumber number)
        const;

    /**
     * @brief
     * Stores contractor's public "key" into internal storage in OPEN form.
     *
     * @throws "ValueError" in case if "number" is 0,
     * or is greatest than "kDefaultKeysSetSize".
     *
     * @throws "ConsistencyError" in case if this contractor
     * already has some key related to record with number = "number".
     *
     * @throws "RuntimeError" in case of any internal error.
     * Writes corresponding log record before exception throwing.
     */
    void setContractorPublicKey(
        IOTransaction::Shared ioTransaction,
        KeyNumber number,
        const lamport::PublicKey::Shared key);

    /**
     * @brief
     * Returns "true" if internal key storage contains at least "count" of
     * unused contractor's PubKeys.
     * Otherwise - returns "false".
     *
     * @throws "RuntimeError" in case of any internal error.
     * Writes corresponding log record before exception throwing.
     */
    bool contractorKeysPresent(
        IOTransaction::Shared ioTransaction);

    bool ownKeysPresent(
        IOTransaction::Shared ioTransaction);

    bool allContractorKeysPresent(
        IOTransaction::Shared ioTransaction);

    bool ownKeysCriticalCount(
        IOTransaction::Shared ioTransaction);

    /**
     * @brief
     * Uses next available private key for signing the data.
     * Right after data signing - private key is cutted to prevent key reuse.
     * Doesn't modifies the data itself.
     *
     * @returns Lamport Sign and number of corresponding PKey,
     * that was used during "data" signing.
     *
     * @throws "KeyError" in case if no available PKey is present.
     * @throws "RuntimeError" in case of any internal error.
     * Writes corresponding log record before exception throwing.
     */
    pair<lamport::Signature::Shared, KeyNumber> sign(
        IOTransaction::Shared ioTransaction,
        BytesShared data,
        const size_t size);

    /**
     * @returns "true" if data was signed by the private key,
     * that is corresponding to the contractor's public key with number "keyNumber".
     * Otherwise returns "false".
     *
     * @throws "KeyError" in case if no available contractor PubKey is present.
     * @throws "RuntimeError" in case of any internal error.
     * Writes corresponding log record before exception throwing.
     */
    bool checkSign(
        IOTransaction::Shared ioTransaction,
        BytesShared data,
        const size_t size,
        const lamport::Signature::Shared signature,
        const KeyNumber keyNumber);

    void removeUnusedContractorKeys(
        IOTransaction::Shared ioTransaction);

    void removeUnusedOwnKeys(
        IOTransaction::Shared ioTransaction);

    bool saveOutgoingPaymentReceipt(
        IOTransaction::Shared ioTransaction,
        const AuditNumber auditNumber,
        const TransactionUUID &transactionUUID,
        const KeyNumber ownPublicKeyNumber,
        const TrustLineAmount &amount,
        const lamport::Signature::Shared signature);

    bool saveIncomingPaymentReceipt(
        IOTransaction::Shared ioTransaction,
        const AuditNumber auditNumber,
        const TransactionUUID &transactionUUID,
        const KeyNumber contractorPublicKeyNumber,
        const TrustLineAmount &amount,
        const Signature::Shared contractorSignature);

    void saveAudit(
        IOTransaction::Shared ioTransaction,
        const AuditNumber auditNumber,
        const KeyNumber ownKeyNumber,
        const lamport::Signature::Shared ownSignature,
        const KeyNumber contractorKeyNumber,
        const lamport::Signature::Shared contractorSignature,
        const TrustLineAmount &incomingAmount,
        const TrustLineAmount &outgoingAmount,
        const TrustLineBalance &balance);

    AuditRecord::Shared actualFullAudit(
        IOTransaction::Shared ioTransaction);

    bool checkOwnConflictedSignature(
        IOTransaction::Shared ioTransaction,
        BytesShared data,
        const size_t size,
        const lamport::Signature::Shared ownSignature,
        lamport::KeyHash::Shared ownKeyHash);

    bool checkContractorConflictedSignature(
        IOTransaction::Shared ioTransaction,
        BytesShared data,
        const size_t size,
        const lamport::Signature::Shared contractorSignature,
        lamport::KeyHash::Shared contractorKeyHash);

    vector<ReceiptRecord::Shared> incomingReceipts(
        IOTransaction::Shared ioTransaction,
        AuditNumber auditNumber);

    vector<ReceiptRecord::Shared> outgoingReceipts(
        IOTransaction::Shared ioTransaction,
        AuditNumber auditNumber);

    bool checkConflictedIncomingReceipt(
        IOTransaction::Shared ioTransaction,
        BytesShared data,
        const size_t size,
        const lamport::Signature::Shared ownSignature,
        lamport::KeyHash::Shared ownKeyHash);

    bool checkConflictedOutgoingReceipt(
        IOTransaction::Shared ioTransaction,
        BytesShared data,
        const size_t size,
        const lamport::Signature::Shared ownSignature,
        lamport::KeyHash::Shared ownKeyHash);

    void acceptAudit(
        IOTransaction::Shared ioTransaction,
        AuditRecord::Shared auditRecord);

    void acceptReceipts(
        IOTransaction::Shared ioTransaction,
        vector<ReceiptRecord::Shared> contractorIncomingReceipts,
        vector<ReceiptRecord::Shared> contractorOutgoingReceipts);

protected:
    /**
     * @brief checks "number" for it's range.
     * @throws "ValueError" if "number" is not in range of valid values.
     */
    void keyNumberGuard(
        const KeyNumber &number)
        const;

    /**
     * @brief checks "data" and "size" for correct values.
     * @throws "ValueError" iin case of any unexpected value.
     */
    void dataGuard(
        const BytesShared data,
        const size_t size)
        const;

private:
    LoggerStream info() const;

    LoggerStream error() const;

    LoggerStream warning() const;

    const string logHeader() const;

private:
    TrustLineID mTrustLineID;
    //todo Encryptor &mEncryptor;
    Logger &mLogger;
};


}

#endif // KEYCHAIN_H
