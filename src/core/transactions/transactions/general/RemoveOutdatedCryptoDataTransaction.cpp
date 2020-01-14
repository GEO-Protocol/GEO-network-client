#include "RemoveOutdatedCryptoDataTransaction.h"

RemoveOutdatedCryptoDataTransaction::RemoveOutdatedCryptoDataTransaction(
    EquivalentsSubsystemsRouter *equivalentsSubsystemsRouter,
    StorageHandler *storageHandler,
    Keystore *keystore,
    Logger &logger):
    BaseTransaction(
        BaseTransaction::RemoveOutdatedCryptoDataType,
        0,
        logger),
    mEquivalentsSubsystemsRouter(equivalentsSubsystemsRouter),
    mStorageHandler(storageHandler),
    mKeysStore(keystore)
{}

TransactionResult::SharedConst RemoveOutdatedCryptoDataTransaction::run()
{
    info() << "run";
    auto ioTransaction = mStorageHandler->beginTransaction();
    for (const auto &equivalent : mEquivalentsSubsystemsRouter->equivalents()) {
        auto trustLinesManager = mEquivalentsSubsystemsRouter->trustLinesManager(
            equivalent);
        for (const auto &contractorIDAndTrustLine : trustLinesManager->trustLines()) {
            auto keyChain = mKeysStore->keychain(
                contractorIDAndTrustLine.second->trustLineID());
            auto currentAuditNumber = keyChain.getCurrentAuditSignatureAndKeyNumber(
                ioTransaction);
            if (currentAuditNumber.second <= kOutdatedDifference) {
                // there are no outdated crypto data
                continue;
            }
            auto outdatedAuditNumber = currentAuditNumber.second - kOutdatedDifference;
            keyChain.removeOutdatedCryptoData(
                ioTransaction,
                outdatedAuditNumber);
        }
    }
    return resultDone();
}

const string RemoveOutdatedCryptoDataTransaction::logHeader() const
{
    stringstream s;
    s << "[RemoveOutdatedCryptoDataTA: " << currentTransactionUUID() << "]";
    return s.str();
}