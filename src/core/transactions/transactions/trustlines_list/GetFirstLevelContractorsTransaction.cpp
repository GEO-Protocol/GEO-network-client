#include "GetFirstLevelContractorsTransaction.h"

GetFirstLevelContractorsTransaction::GetFirstLevelContractorsTransaction(
    NodeUUID &nodeUUID,
    GetFirstLevelContractorsCommand::Shared command,
    TrustLinesManager *manager,
    Logger &logger)
    noexcept :
    BaseTransaction(
        BaseTransaction::ContractorsList,
        nodeUUID,
        command->equivalent(),
        logger),
    mCommand(command),
    mTrustLinesManager(manager)
{}

GetFirstLevelContractorsCommand::Shared GetFirstLevelContractorsTransaction::command() const
{
    return mCommand;
}

TransactionResult::SharedConst GetFirstLevelContractorsTransaction::run()
{
    const auto kNeighborsCount = mTrustLinesManager->trustLines().size();
    stringstream ss;
    ss << to_string(kNeighborsCount);
    for (const auto &kNodeUUIDAndTrustline: mTrustLinesManager->trustLines()) {
        ss << kTokensSeparator;
        ss << kNodeUUIDAndTrustline.first;
    }
    ss << kCommandsSeparator;
    string kResultInfo = ss.str();
    return transactionResultFromCommand(mCommand->resultOk(kResultInfo));
}

const string GetFirstLevelContractorsTransaction::logHeader() const
{
    stringstream s;
    s << "[GetFirstLevelContractorsTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}
