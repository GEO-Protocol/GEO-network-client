#include "RemoveOutdatedCryptoDataTransaction.h"

RemoveOutdatedCryptoDataTransaction::RemoveOutdatedCryptoDataTransaction(
    RemoveOutdatedCryptoDataCommand::Shared command,
    EquivalentsSubsystemsRouter *equivalentsSubsystemsRouter,
    StorageHandler *storageHandler,
    Keystore *keystore,
    Logger &logger):
    BaseTransaction(
        BaseTransaction::RemoveOutdatedCryptoDataType,
        0,
        logger),
    mCommand(command),
    mEquivalentsSubsystemsRouter(equivalentsSubsystemsRouter),
    mStorageHandler(storageHandler),
    mKeysStore(keystore)
{}

TransactionResult::SharedConst RemoveOutdatedCryptoDataTransaction::run()
{
    info() << "run";
    for (const auto &equivalent : mEquivalentsSubsystemsRouter->equivalents()) {
        auto ioTransaction = mStorageHandler->beginTransaction();
        try {
            info() << "Equivalent " << equivalent;
            auto trustLinesManager = mEquivalentsSubsystemsRouter->trustLinesManager(
                equivalent);
            for (const auto &contractorIDAndTrustLine : trustLinesManager->trustLines()) {
                info() << "Trust Line ID " << contractorIDAndTrustLine.second->trustLineID();
                auto keyChain = mKeysStore->keychain(
                    contractorIDAndTrustLine.second->trustLineID());
                auto currentAuditRecord = ioTransaction->auditHandler()->getActualAuditFull(
                    contractorIDAndTrustLine.second->trustLineID());
                info() << "Current audit number " << currentAuditRecord->auditNumber();
                if (currentAuditRecord->auditNumber() <= kOutdatedDifference) {
                    info() << "No outdated crypto data";
                    continue;
                }
                auto outdatedAuditNumber = currentAuditRecord->auditNumber() - kOutdatedDifference;
                info() << "Outdated audit number " << outdatedAuditNumber;
                keyChain.removeOutdatedCryptoData(
                    ioTransaction,
                    outdatedAuditNumber);
                info() << "Outdated crypto data successfully deleted";
            }
        } catch (IOError &e) {
            ioTransaction->rollback();
            warning() << "Can't remove outdated crypto data";
        }
    }
    mStorageHandler->vacuum();
    return resultDone();
}

TransactionResult::SharedConst RemoveOutdatedCryptoDataTransaction::resultOK()
{
    return transactionResultFromCommand(
        mCommand->responseOK());
}

const string RemoveOutdatedCryptoDataTransaction::logHeader() const
{
    stringstream s;
    s << "[RemoveOutdatedCryptoDataTA: " << currentTransactionUUID() << "]";
    return s.str();
}