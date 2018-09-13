#include "GetFirstLevelContractorsBalancesTransaction.h"

GetFirstLevelContractorsBalancesTransaction::GetFirstLevelContractorsBalancesTransaction(
    NodeUUID &nodeUUID,
    GetTrustLinesCommand::Shared command,
    TrustLinesManager *manager,
    Logger &logger)
    noexcept:
    BaseTransaction(
        BaseTransaction::TrustLinesList,
        nodeUUID,
        command->equivalent(),
        logger),
    mCommand(command),
    mTrustLinesManager(manager)
{}

GetTrustLinesCommand::Shared GetFirstLevelContractorsBalancesTransaction::command() const
{
    return mCommand;
}

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
        for (const auto &kNodeUUIDAndTrustLine: mTrustLinesManager->trustLines()) {
            if (recordIdx < mCommand->from()) {
                recordIdx++;
                continue;
            }
            recordIdx++;
            ss << kTokensSeparator;
            ss << kNodeUUIDAndTrustLine.first;
            ss << kTokensSeparator;
            ss << kNodeUUIDAndTrustLine.second->incomingTrustAmount();
            ss << kTokensSeparator;
            ss << kNodeUUIDAndTrustLine.second->outgoingTrustAmount();
            ss << kTokensSeparator;
            ss << kNodeUUIDAndTrustLine.second->balance();
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
