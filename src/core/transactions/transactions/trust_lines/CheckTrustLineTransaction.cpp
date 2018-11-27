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

CheckTrustLineTransaction::CheckTrustLineTransaction(
    const NodeUUID &nodeUUID,
    const SerializedEquivalent equivalent,
    const NodeUUID &contractorUUID,
    ContractorID contractorID,
    bool isActionInitiator,
    TrustLinesManager *manager,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::CheckTrustLineAfterPaymentTransactionType,
        nodeUUID,
        equivalent,
        logger),
    mContractorUUID(contractorUUID),
    mContractorID(contractorID),
    mIsActionInitiator(isActionInitiator),
    mTrustLinesManager(manager)
{}

TransactionResult::SharedConst CheckTrustLineTransaction::run()
{
    info() << "run " << mContractorUUID << " " << mContractorID;
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
            auditNewSignal(
                mContractorUUID,
                mContractorID,
                mEquivalent);
            break;
        }
        case TrustLinesManager::KeysSharing: {
            info() << "Keys sharing action";
            publicKeysSharingNewSignal(
                mContractorUUID,
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
    noexcept
{
    stringstream s;
    s << "[CheckTrustLineTransaction: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}