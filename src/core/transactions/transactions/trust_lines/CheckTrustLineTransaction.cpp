#include "CheckTrustLineTransaction.h"

CheckTrustLineTransaction::CheckTrustLineTransaction(
    const NodeUUID &nodeUUID,
    const SerializedEquivalent equivalent,
    const NodeUUID &contractorUUID,
    bool isActionInitiator,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::CheckTrustLineAfterPaymentTransactionType,
        nodeUUID,
        equivalent,
        logger),
    mContractorUUID(contractorUUID),
    mIsActionInitiator(isActionInitiator),
    mTrustLinesManager(manager),
    mStorageHandler(storageHandler),
    mKeysStore(keystore),
    mTrustLinesInfluenceController(trustLinesInfluenceController)
{}

TransactionResult::SharedConst CheckTrustLineTransaction::run()
{
    info() << "run";
    auto action = mTrustLinesManager->checkTrustLineAfterTransaction(
        mContractorUUID,
        mIsActionInitiator);
    switch (action) {
        case TrustLinesManager::Audit: {
            info() << "Audit action";
            launchSubsidiaryTransaction(
                make_shared<AuditSourceTransaction>(
                    mNodeUUID,
                    mContractorUUID,
                    mEquivalent,
                    mTrustLinesManager,
                    mStorageHandler,
                    mKeysStore,
                    mTrustLinesInfluenceController,
                    mLog));
            break;
        }
        case TrustLinesManager::KeysSharing: {
            info() << "Keys sharing action";
            launchSubsidiaryTransaction(
                make_shared<PublicKeysSharingSourceTransaction>(
                    mNodeUUID,
                    mContractorUUID,
                    mEquivalent,
                    mTrustLinesManager,
                    mStorageHandler,
                    mKeysStore,
                    mTrustLinesInfluenceController,
                    mLog));
            break;
        }
        default: {
            info() << "No actions";
        }
    }
    return resultDone();
}

const string CheckTrustLineTransaction::logHeader() const
    noexcept
{
    stringstream s;
    s << "[CheckTrustLineOnAuditOrKeysSharingTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}