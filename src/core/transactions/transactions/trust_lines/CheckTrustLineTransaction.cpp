#include "CheckTrustLineTransaction.h"

CheckTrustLineTransaction::CheckTrustLineTransaction(
    const SerializedEquivalent equivalent,
    ContractorID contractorID,
    bool isActionInitiator,
    TrustLinesManager *manager,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::CheckTrustLineAfterPaymentTransactionType,
        equivalent,
        logger),
    mContractorID(contractorID),
    mIsActionInitiator(isActionInitiator),
    mTrustLinesManager(manager)
{}

TransactionResult::SharedConst CheckTrustLineTransaction::run()
{
    info() << "run " << mContractorID;
    if (mTrustLinesManager->trustLineState(mContractorID) == TrustLine::Archived) {
        info() << "TL is Archived";
        return resultDone();
    }
    auto action = mTrustLinesManager->checkTrustLineAfterTransaction(
        mContractorID,
        mIsActionInitiator);
    switch (action) {
        case TrustLinesManager::Audit: {
            info() << "Audit action";
            auditSignal(
                mContractorID,
                mEquivalent);
            break;
        }
        case TrustLinesManager::KeysSharing: {
            info() << "Keys sharing action";
            publicKeysSharingSignal(
                mContractorID,
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
{
    stringstream s;
    s << "[CheckTrustLineTransaction: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}