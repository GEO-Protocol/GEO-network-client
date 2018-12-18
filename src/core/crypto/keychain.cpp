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
        return {
            trustLineID,
            // todo mEncryptor,
            mLogger};
    }

    lamport::PublicKey::Shared Keystore::generateAndSaveKeyPairForPaymentTransaction(
        IOTransaction::Shared ioTransaction,
        const TransactionUUID &transactionUUID)
    {
        lamport::PrivateKey pKey;
        auto pubKey = pKey.derivePublicKey();
        ioTransaction->paymentKeysHandler()->saveOwnKey(
            transactionUUID,
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

    lamport::PublicKey::Shared TrustLineKeychain::publicKey(
        IOTransaction::Shared ioTransaction,
        const KeyNumber number) const
    {
        keyNumberGuard(number);

        try {
            auto publicKey = ioTransaction->ownKeysHandler()->getPublicKey(
                mTrustLineID,
                number);
            return publicKey;
        } catch (NotFoundError &e) {
            return nullptr;
        } catch (IOError &e) {
            error() << "Can't get public key " << number << ". Details: " << e.what();
            throw e;
        }
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

    bool TrustLineKeychain::contractorKeysPresent(
        IOTransaction::Shared ioTransaction)
    {
        return ioTransaction->contractorKeysHandler()->availableKeysCnt(mTrustLineID) > 0;
    }

    bool TrustLineKeychain::ownKeysPresent(
        IOTransaction::Shared ioTransaction)
    {
        return ioTransaction->ownKeysHandler()->availableKeysCnt(mTrustLineID) > 0;
    }

    bool TrustLineKeychain::allContractorKeysPresent(
        IOTransaction::Shared ioTransaction,
        KeysCount contractorKeysCount)
    {
        return ioTransaction->contractorKeysHandler()->availableKeysCnt(mTrustLineID) == contractorKeysCount;
    }

    bool TrustLineKeychain::ownKeysCriticalCount(
        IOTransaction::Shared ioTransaction)
    {
        return ioTransaction->ownKeysHandler()->availableKeysCnt(mTrustLineID) <= kMinKeysSetSize;
    }

    pair<lamport::Signature::Shared, KeyNumber> TrustLineKeychain::sign(
        IOTransaction::Shared ioTransaction,
        BytesShared data,
        const std::size_t size)
    {
        dataGuard(data, size);

        pair<PrivateKey*, KeyNumber> privateKeyAndNumber;
        try {
            privateKeyAndNumber = ioTransaction->ownKeysHandler()->nextAvailableKey(
                mTrustLineID);
            // todo: decrypt private key.
            // todo: throw KeyError if no key is available;
        } catch (NotFoundError &e) {
            warning() << "Can't get available private key for TL " << mTrustLineID;
            throw e;
        }

        auto signature = make_shared<lamport::Signature>(
            data.get(),
            size,
            privateKeyAndNumber.first);

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

    void TrustLineKeychain::removeUnusedContractorKeys(
        IOTransaction::Shared ioTransaction)
    {
        ioTransaction->contractorKeysHandler()->removeUnusedKeys(
            mTrustLineID);
    }

    void TrustLineKeychain::removeUnusedOwnKeys(
        IOTransaction::Shared ioTransaction)
    {
        ioTransaction->ownKeysHandler()->removeUnusedKeys(
            mTrustLineID);
    }

    bool TrustLineKeychain::saveOutgoingPaymentReceipt(
        IOTransaction::Shared ioTransaction,
        const AuditNumber auditNumber,
        const TransactionUUID &transactionUUID,
        const KeyNumber ownKeyNumber,
        const TrustLineAmount &amount,
        const lamport::Signature::Shared signature)
    {
        try {
            auto contractorKeyHash = ioTransaction->ownKeysHandler()->getPublicKeyHash(
                mTrustLineID,
                ownKeyNumber);

            ioTransaction->outgoingPaymentReceiptHandler()->saveRecord(
                mTrustLineID,
                auditNumber,
                transactionUUID,
                contractorKeyHash,
                amount);

            ioTransaction->ownKeysHandler()->invalidKey(
                mTrustLineID,
                ownKeyNumber,
                signature);
        } catch (NotFoundError &e) {
            warning() << "There are no valid own key with number " << ownKeyNumber;
            return false;
        } catch (IOError &e) {
            warning() << "Can't save outgoing receipt into storage. Details: " << e.what();
            return false;
        }
        return true;
    }

    bool TrustLineKeychain::saveIncomingPaymentReceipt(
        IOTransaction::Shared ioTransaction,
        const AuditNumber auditNumber,
        const TransactionUUID &transactionUUID,
        const KeyNumber contractorKeyNumber,
        const TrustLineAmount &amount,
        const Signature::Shared contractorSignature)
    {
        try {
            auto contractorKeyHash = ioTransaction->contractorKeysHandler()->keyHashByNumber(
                mTrustLineID,
                contractorKeyNumber);

            ioTransaction->incomingPaymentReceiptHandler()->saveRecord(
                mTrustLineID,
                auditNumber,
                transactionUUID,
                contractorKeyHash,
                amount,
                contractorSignature);

            ioTransaction->contractorKeysHandler()->invalidKey(
                mTrustLineID,
                contractorKeyNumber);
        } catch (NotFoundError &e) {
            warning() << "There are no valid contractor key with number " << contractorKeyNumber;
            return false;
        } catch (IOError &e) {
            warning() << "Can't save incoming receipt into storage. Details: " << e.what();
            return false;
        }
        return true;
    }

    void TrustLineKeychain::saveFullAudit(
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

        ioTransaction->auditHandler()->saveFullAudit(
            auditNumber,
            mTrustLineID,
            ownKeyHash,
            ownSignature,
            contractorKeyHash,
            contractorSignature,
            incomingAmount,
            outgoingAmount,
            balance);

        ioTransaction->ownKeysHandler()->invalidKey(
            mTrustLineID,
            ownKeyNumber,
            ownSignature);

        ioTransaction->contractorKeysHandler()->invalidKey(
            mTrustLineID,
            contractorKeyNumber);
    }

    void TrustLineKeychain::saveOwnAuditPart(
        IOTransaction::Shared ioTransaction,
        const AuditNumber auditNumber,
        const KeyNumber ownKeyNumber,
        const lamport::Signature::Shared ownSignature,
        const TrustLineAmount &incomingAmount,
        const TrustLineAmount &outgoingAmount,
        const TrustLineBalance &balance)
    {
        auto ownKeyHash = ioTransaction->ownKeysHandler()->getPublicKeyHash(
            mTrustLineID,
            ownKeyNumber);

        ioTransaction->auditHandler()->saveOwnAuditPart(
            auditNumber,
            mTrustLineID,
            ownKeyHash,
            ownSignature,
            incomingAmount,
            outgoingAmount,
            balance);

        ioTransaction->ownKeysHandler()->invalidKey(
            mTrustLineID,
            ownKeyNumber,
            ownSignature);
    }

    void TrustLineKeychain::saveContractorAuditPart(
        IOTransaction::Shared ioTransaction,
        const AuditNumber auditNumber,
        const KeyNumber contractorKeyNumber,
        const lamport::Signature::Shared contractorSignature)
    {
        auto contractorKeyHash = ioTransaction->contractorKeysHandler()->keyHashByNumber(
            mTrustLineID,
            contractorKeyNumber);

        ioTransaction->auditHandler()->saveContractorAuditPart(
            auditNumber,
            mTrustLineID,
            contractorKeyHash,
            contractorSignature);

        ioTransaction->contractorKeysHandler()->invalidKey(
            mTrustLineID,
            contractorKeyNumber);
    }

    pair<lamport::Signature::Shared, KeyNumber> TrustLineKeychain::getSignatureAndKeyNumberForPendingAudit(
        IOTransaction::Shared ioTransaction,
        const AuditNumber auditNumber)
    {
        auto auditRecord = ioTransaction->auditHandler()->getActualAuditFull(
            mTrustLineID);
        if (auditRecord->auditNumber() != auditNumber) {
            warning() << "Audit numbers are different: number from memory " << auditNumber
                      << " number from storage " << auditRecord->auditNumber();
            throw ValueError("Invalid audit number");
        }
        if (auditRecord->contractorSignature() != nullptr) {
            throw ValueError("Not empty contractor signature");
        }

        try {
            auto ownKeyNumber = ioTransaction->ownKeysHandler()->getKeyNumberByHash(
                auditRecord->ownKeyHash());
            return make_pair(
                auditRecord->ownSignature(),
                ownKeyNumber);
        } catch (NotFoundError &e) {
            throw ValueError("Can't get key number. Details: " + e.message());
        }
    }

    TrustLineAmount TrustLineKeychain::incomingCommittedReceiptsAmountsSum(
        IOTransaction::Shared ioTransaction,
        const AuditNumber auditNumber)
    {
        TrustLineAmount result = TrustLine::kZeroAmount();
        auto incomingReceiptsAmounts = ioTransaction->incomingPaymentReceiptHandler()->auditAmounts(
            mTrustLineID,
            auditNumber);
        for (const auto &incomingReceiptAmount : incomingReceiptsAmounts) {
            if (ioTransaction->paymentParticipantsVotesHandler()->isTransactionCommitted(incomingReceiptAmount.first)) {
                result = result + incomingReceiptAmount.second;
            }
        }
        return result;
    }

    TrustLineAmount TrustLineKeychain::outgoingCommittedReceiptsAmountsSum(
        IOTransaction::Shared ioTransaction,
        const AuditNumber auditNumber)
    {
        TrustLineAmount result = TrustLine::kZeroAmount();
        auto outgoingReceiptsAmounts = ioTransaction->outgoingPaymentReceiptHandler()->auditAmounts(
            mTrustLineID,
            auditNumber);
        for (const auto &outgoingReceiptAmount : outgoingReceiptsAmounts) {
            if (ioTransaction->paymentParticipantsVotesHandler()->isTransactionCommitted(outgoingReceiptAmount.first)) {
                result = result + outgoingReceiptAmount.second;
            }
        }
        return result;
    }

    AuditRecord::Shared TrustLineKeychain::actualFullAudit(
        IOTransaction::Shared ioTransaction)
    {
        return ioTransaction->auditHandler()->getActualAuditFull(
            mTrustLineID);
    }

    bool TrustLineKeychain::checkOwnConflictedSignature(
        IOTransaction::Shared ioTransaction,
        BytesShared data,
        const size_t size,
        const lamport::Signature::Shared ownSignature,
        lamport::KeyHash::Shared ownKeyHash)
    {
        auto publicKey = ioTransaction->ownKeysHandler()->getPublicKeyByHash(
            mTrustLineID,
            ownKeyHash);
        return ownSignature->check(
            data.get(),
            size,
            publicKey);
    }

    bool TrustLineKeychain::checkContractorConflictedSignature(
        IOTransaction::Shared ioTransaction,
        BytesShared data,
        const size_t size,
        const lamport::Signature::Shared contractorSignature,
        lamport::KeyHash::Shared contractorKeyHash)
    {
        auto publicKey = ioTransaction->contractorKeysHandler()->keyByHash(
            mTrustLineID,
            contractorKeyHash);
        return contractorSignature->check(
            data.get(),
            size,
            publicKey);
    }

    vector<ReceiptRecord::Shared> TrustLineKeychain::incomingReceipts(
        IOTransaction::Shared ioTransaction,
        AuditNumber auditNumber)
    {
        return ioTransaction->incomingPaymentReceiptHandler()->receiptsByAuditNumber(
            mTrustLineID,
            auditNumber);
    }

    vector<ReceiptRecord::Shared> TrustLineKeychain::outgoingReceipts(
        IOTransaction::Shared ioTransaction,
        AuditNumber auditNumber)
    {
        return ioTransaction->outgoingPaymentReceiptHandler()->receiptsByAuditNumber(
            mTrustLineID,
            auditNumber);
    }

    bool TrustLineKeychain::checkConflictedIncomingReceipt(
            IOTransaction::Shared ioTransaction,
            BytesShared data,
            const size_t size,
            const lamport::Signature::Shared ownSignature,
            lamport::KeyHash::Shared ownKeyHash)
    {
        auto publicKey = ioTransaction->ownKeysHandler()->getPublicKeyByHash(
            mTrustLineID,
            ownKeyHash);
        return ownSignature->check(
            data.get(),
            size,
            publicKey);
    }

    bool TrustLineKeychain::checkConflictedOutgoingReceipt(
        IOTransaction::Shared ioTransaction,
        BytesShared data,
        const size_t size,
        const lamport::Signature::Shared ownSignature,
        lamport::KeyHash::Shared ownKeyHash)
    {
        auto publicKey = ioTransaction->contractorKeysHandler()->keyByHash(
            mTrustLineID,
            ownKeyHash);
        return ownSignature->check(
            data.get(),
            size,
            publicKey);
    }

    void TrustLineKeychain::acceptAudit(
        IOTransaction::Shared ioTransaction,
        AuditRecord::Shared auditRecord)
    {
        ioTransaction->auditHandler()->saveFullAudit(
                auditRecord->auditNumber(),
                mTrustLineID,
                auditRecord->contractorKeyHash(),
                auditRecord->contractorSignature(),
                auditRecord->ownKeyHash(),
                auditRecord->ownSignature(),
                auditRecord->outgoingAmount(),
                auditRecord->incomingAmount(),
                auditRecord->balance() * (-1));

        ioTransaction->ownKeysHandler()->invalidKeyByHash(
            mTrustLineID,
            auditRecord->contractorKeyHash(),
            auditRecord->contractorSignature());

        ioTransaction->contractorKeysHandler()->invalidKeyByHash(
            mTrustLineID,
            auditRecord->ownKeyHash());
    }

    void TrustLineKeychain::acceptReceipts(
        IOTransaction::Shared ioTransaction,
        vector<ReceiptRecord::Shared> contractorIncomingReceipts,
        vector<ReceiptRecord::Shared> contractorOutgoingReceipts)
    {
        for (const auto &contractorIncomingReceipt : contractorIncomingReceipts) {
            ioTransaction->outgoingPaymentReceiptHandler()->saveRecord(
                mTrustLineID,
                contractorIncomingReceipt->auditNumber(),
                contractorIncomingReceipt->transactionUUID(),
                contractorIncomingReceipt->keyHash(),
                contractorIncomingReceipt->amount());

            ioTransaction->ownKeysHandler()->invalidKeyByHash(
                mTrustLineID,
                contractorIncomingReceipt->keyHash(),
                contractorIncomingReceipt->signature());
        }

        for (const auto &contractorOutgoingReceipt : contractorOutgoingReceipts) {
            ioTransaction->incomingPaymentReceiptHandler()->saveRecord(
                mTrustLineID,
                contractorOutgoingReceipt->auditNumber(),
                contractorOutgoingReceipt->transactionUUID(),
                contractorOutgoingReceipt->keyHash(),
                contractorOutgoingReceipt->amount(),
                contractorOutgoingReceipt->signature());

            ioTransaction->contractorKeysHandler()->invalidKeyByHash(
                mTrustLineID,
                contractorOutgoingReceipt->keyHash());
        }
    }

    pair<lamport::Signature::Shared, KeyNumber> TrustLineKeychain::getCurrentAuditSignatureAndKeyNumber(
        IOTransaction::Shared ioTransaction)
    {
        auto auditRecord = ioTransaction->auditHandler()->getActualAuditFull(
            mTrustLineID);

        try {
            auto ownKeyNumber = ioTransaction->ownKeysHandler()->getKeyNumberByHash(
                auditRecord->ownKeyHash());
            return make_pair(
                auditRecord->ownSignature(),
                ownKeyNumber);
        } catch (NotFoundError &e) {
            throw ValueError("Can't get key number. Details: " + e.message());
        }
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

    LoggerStream TrustLineKeychain::error() const
    {
        return mLogger.error(logHeader());
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
