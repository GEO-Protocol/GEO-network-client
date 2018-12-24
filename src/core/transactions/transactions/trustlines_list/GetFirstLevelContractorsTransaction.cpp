#include "GetFirstLevelContractorsTransaction.h"

GetFirstLevelContractorsTransaction::GetFirstLevelContractorsTransaction(
    GetFirstLevelContractorsCommand::Shared command,
    TrustLinesManager *manager,
    Logger &logger)
    noexcept :
    BaseTransaction(
        BaseTransaction::ContractorsList,
        command->equivalent(),
        logger),
    mCommand(command),
    mTrustLinesManager(manager)
{}

TransactionResult::SharedConst GetFirstLevelContractorsTransaction::run()
{
    const auto kNeighborsCount = mTrustLinesManager->trustLines().size();
    stringstream ss;
    ss << to_string(kNeighborsCount);
    for (const auto &kNodeIDAndTrustLine: mTrustLinesManager->trustLines()) {
        ss << kTokensSeparator;
        ss << kNodeIDAndTrustLine.first;
    }
    ss << kCommandsSeparator;
    string kResultInfo = ss.str();
    return transactionResultFromCommand(
        mCommand->resultOk(kResultInfo));
}

const string GetFirstLevelContractorsTransaction::logHeader() const
{
    stringstream s;
    s << "[GetFirstLevelContractorsTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}
