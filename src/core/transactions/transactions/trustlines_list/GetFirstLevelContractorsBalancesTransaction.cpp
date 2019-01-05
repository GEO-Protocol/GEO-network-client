#include "GetFirstLevelContractorsBalancesTransaction.h"

GetFirstLevelContractorsBalancesTransaction::GetFirstLevelContractorsBalancesTransaction(
    GetTrustLinesCommand::Shared command,
    TrustLinesManager *manager,
    Logger &logger)
    noexcept:
    BaseTransaction(
        BaseTransaction::TrustLinesList,
        command->equivalent(),
        logger),
    mCommand(command),
    mTrustLinesManager(manager)
{}

TransactionResult::SharedConst GetFirstLevelContractorsBalancesTransaction::run()
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
            ss << kTokensSeparator;
            ss << kNodeIDAndTrustLine.first;
            ss << kTokensSeparator;
            ss << kNodeIDAndTrustLine.second->state();
            ss << kTokensSeparator;
            ss << kNodeIDAndTrustLine.second->isOwnKeysPresent();
            ss << kTokensSeparator;
            ss << kNodeIDAndTrustLine.second->isContractorKeysPresent();
            ss << kTokensSeparator;
            ss << kNodeIDAndTrustLine.second->incomingTrustAmount();
            ss << kTokensSeparator;
            ss << kNodeIDAndTrustLine.second->outgoingTrustAmount();
            ss << kTokensSeparator;
            ss << kNodeIDAndTrustLine.second->balance();
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

const string GetFirstLevelContractorsBalancesTransaction::logHeader() const
{
    stringstream s;
    s << "[GetFirstLevelContractorsBalancesTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}
