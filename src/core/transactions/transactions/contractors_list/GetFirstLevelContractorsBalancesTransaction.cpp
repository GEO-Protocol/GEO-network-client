#include "GetFirstLevelContractorsBalancesTransaction.h"

GetFirstLevelContractorsBalancesTransaction::GetFirstLevelContractorsBalancesTransaction(
        NodeUUID &nodeUUID,
        GetFirstLevelContractorsBalancesCommand::Shared command,
        TrustLinesManager *manager,
        Logger *logger):
        BaseTransaction(
                BaseTransaction::TransactionType::ContractosBalancesListTransaction,
                nodeUUID,
                logger),
        mCommand(command),
        mTrustLinesManager(manager)
{}

GetFirstLevelContractorsBalancesCommand::Shared GetFirstLevelContractorsBalancesTransaction::command() const {
    return mCommand;
}

TransactionResult::SharedConst GetFirstLevelContractorsBalancesTransaction::run() {
    const auto neighbors_count = mTrustLinesManager->trustLines().size();
    stringstream ss;
    ss << to_string(neighbors_count) << "\t";
    for (auto kNodeUUIDAndTrustline: mTrustLinesManager->trustLines()) {
        ss << kNodeUUIDAndTrustline.first << "\t";
        ss << kNodeUUIDAndTrustline.second->incomingTrustAmount() << "\t";
        ss << kNodeUUIDAndTrustline.second->outgoingTrustAmount() << "\t";
        ss << kNodeUUIDAndTrustline.second->balance() << "\t";
    }
    ss << "\n";
    string kResultInfo = ss.str();
    cout << kResultInfo << endl;
    return transactionResultFromCommand(mCommand->resultOk(kResultInfo));
}
