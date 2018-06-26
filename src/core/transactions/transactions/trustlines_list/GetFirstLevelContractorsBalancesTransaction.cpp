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
    ss << to_string(kNeighborsCount);
    for (const auto &kNodeUUIDAndTrustline: mTrustLinesManager->trustLines()) {
        // todo discuss if exclude non active TLs
        ss << kTokensSeparator;
        ss << kNodeUUIDAndTrustline.first;
        ss << kTokensSeparator;
        ss << kNodeUUIDAndTrustline.second->incomingTrustAmount();
        ss << kTokensSeparator;
        ss << kNodeUUIDAndTrustline.second->outgoingTrustAmount();
        ss << kTokensSeparator;
        ss << kNodeUUIDAndTrustline.second->balance();
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
