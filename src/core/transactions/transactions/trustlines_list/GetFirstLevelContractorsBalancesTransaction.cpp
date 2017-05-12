#include "GetFirstLevelContractorsBalancesTransaction.h"

GetFirstLevelContractorsBalancesTransaction::GetFirstLevelContractorsBalancesTransaction(
    NodeUUID &nodeUUID,
    GetTrustLinesCommand::Shared command,
    TrustLinesManager *manager,
    Logger *logger)
    noexcept:
    BaseTransaction(
        BaseTransaction::TrustlinesList,
        nodeUUID,
        logger),
    mCommand(command),
    mTrustLinesManager(manager)
{}

GetTrustLinesCommand::Shared GetFirstLevelContractorsBalancesTransaction::command() const {
    return mCommand;
}

TransactionResult::SharedConst GetFirstLevelContractorsBalancesTransaction::run() {
    const auto kNeighborsCount = mTrustLinesManager->trustLines().size();
    stringstream ss;
    ss << to_string(kNeighborsCount) << "\t";
    for (const auto kNodeUUIDAndTrustline: mTrustLinesManager->trustLines()) {
        ss << kNodeUUIDAndTrustline.first << "\t";
        ss << kNodeUUIDAndTrustline.second->incomingTrustAmount() << "\t";
        ss << kNodeUUIDAndTrustline.second->outgoingTrustAmount() << "\t";
        ss << kNodeUUIDAndTrustline.second->balance() << "\t";
    }
    ss << "\n";
    string kResultInfo = ss.str();
    return transactionResultFromCommand(
        mCommand->resultOk(
            kResultInfo));
}
