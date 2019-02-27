#include "GetTrustLinesListTransaction.h"

GetTrustLinesListTransaction::GetTrustLinesListTransaction(
    GetTrustLinesCommand::Shared command,
    TrustLinesManager *trustLinesManager,
    ContractorsManager *contractorsManager,
    Logger &logger)
    noexcept:
    BaseTransaction(
        BaseTransaction::TrustLinesList,
        command->equivalent(),
        logger),
    mCommand(command),
    mTrustLinesManager(trustLinesManager),
    mContractorsManager(contractorsManager)
{}

TransactionResult::SharedConst GetTrustLinesListTransaction::run()
{
    const auto kNeighborsCount = mTrustLinesManager->trustLines().size();
    stringstream ss;
    if (mCommand->from() > kNeighborsCount - 1) {
        ss << "0";
    } else {
        // todo discuss if exclude non active TLs
        auto resultRecordsCount = min(mCommand->count(), kNeighborsCount - mCommand->from());
        ss << to_string(resultRecordsCount);
        size_t recordIdx = 0;
        size_t currentRecordsCount = 0;
        for (const auto &kNodeIDAndTrustLine: mTrustLinesManager->trustLines()) {
            if (recordIdx < mCommand->from()) {
                recordIdx++;
                continue;
            }
            recordIdx++;
            ss << kTokensSeparator << kNodeIDAndTrustLine.first
               << kTokensSeparator << mContractorsManager->contractor(kNodeIDAndTrustLine.first)->outputString()
               << kTokensSeparator << kNodeIDAndTrustLine.second->state()
               << kTokensSeparator << kNodeIDAndTrustLine.second->isOwnKeysPresent()
               << kTokensSeparator << kNodeIDAndTrustLine.second->isContractorKeysPresent()
               << kTokensSeparator << kNodeIDAndTrustLine.second->incomingTrustAmount()
               << kTokensSeparator << kNodeIDAndTrustLine.second->outgoingTrustAmount()
               << kTokensSeparator << kNodeIDAndTrustLine.second->balance();
            currentRecordsCount++;
            if (currentRecordsCount == mCommand->count()) {
                break;
            }
        }
    }
    ss << kCommandsSeparator;
    string kResultInfo = ss.str();
    return transactionResultFromCommand(
        mCommand->resultOk(
            kResultInfo));
}

const string GetTrustLinesListTransaction::logHeader() const
{
    stringstream s;
    s << "[GetTrustLinesListTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}
