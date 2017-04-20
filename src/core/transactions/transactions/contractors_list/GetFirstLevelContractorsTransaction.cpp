#include "GetFirstLevelContractorsTransaction.h"

GetFirstLevelContractorsTransaction::GetFirstLevelContractorsTransaction(NodeUUID &nodeUUID,
                                                                         GetFirstLevelContractorsCommand::Shared command,
                                                                         TrustLinesManager *manager,
                                                                         Logger *logger):
        BaseTransaction(
                BaseTransaction::TransactionType::ContractorsListTransaction,
                nodeUUID,
                logger),
        mCommand(command),
        mTrustLinesManager(manager)
{}

GetFirstLevelContractorsCommand::Shared GetFirstLevelContractorsTransaction::command() const {
    return mCommand;
}

TransactionResult::SharedConst GetFirstLevelContractorsTransaction::run() {
    const auto neighbors_count = mTrustLinesManager->trustLines().size();
    stringstream ss;
    ss << to_string(neighbors_count) << "\t";
    for (auto kNodeUUIDAndTrustline: mTrustLinesManager->trustLines())
        ss << kNodeUUIDAndTrustline.first << "\t";
    ss << "\n";
    string kResultInfo = ss.str();
    return transactionResultFromCommand(mCommand->resultOk(kResultInfo));
}
