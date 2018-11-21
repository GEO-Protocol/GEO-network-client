#include "CheckTrustLineTransaction.h"

CheckTrustLineTransaction::CheckTrustLineTransaction(
    const NodeUUID &nodeUUID,
    const SerializedEquivalent equivalent,
    const NodeUUID &contractorUUID,
    bool isActionInitiator,
    TrustLinesManager *manager,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::CheckTrustLineAfterPaymentTransactionType,
        nodeUUID,
        equivalent,
        logger),
    mContractorUUID(contractorUUID),
    mIsActionInitiator(isActionInitiator),
    mTrustLinesManager(manager)
{}

TransactionResult::SharedConst CheckTrustLineTransaction::run()
{
    info() << "run " << mContractorUUID;
    if (mTrustLinesManager->trustLineState(mContractorUUID) == TrustLine::Archived) {
        info() << "TL is Archived";
        return resultDone();
    }
    auto action = mTrustLinesManager->checkTrustLineAfterTransaction(
        mContractorUUID,
        mIsActionInitiator);
    switch (action) {
        case TrustLinesManager::Audit: {
            info() << "Audit action";
            auditSignal(
                mContractorUUID,
                mEquivalent);
            break;
        }
        case TrustLinesManager::KeysSharing: {
            info() << "Keys sharing action";
            publicKeysSharingSignal(
                mContractorUUID,
                mEquivalent);
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
    s << "[CheckTrustLineTransaction: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}