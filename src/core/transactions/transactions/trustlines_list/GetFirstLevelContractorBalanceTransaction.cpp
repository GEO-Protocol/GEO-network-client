#include "GetFirstLevelContractorBalanceTransaction.h"

GetFirstLevelContractorBalanceTransaction::GetFirstLevelContractorBalanceTransaction(
    NodeUUID &nodeUUID,
    GetTrustLineCommand::Shared command,
    TrustLinesManager *manager,
    Logger &logger)
noexcept:
    BaseTransaction(
        BaseTransaction::TrustLineOne,
        nodeUUID,
        logger),
    mCommand(command),
    mTrustLinesManager(manager)
{}

GetTrustLineCommand::Shared GetFirstLevelContractorBalanceTransaction::command() const {
    return mCommand;
}

TransactionResult::SharedConst GetFirstLevelContractorBalanceTransaction::run() {
    stringstream ss;
    auto contractorUUID = mCommand->contractorUUID();
    if (!mTrustLinesManager->isNeighbor(contractorUUID)) {
        return resultTrustLineIsAbsent();
    }
    auto kContractorTrustLine = mTrustLinesManager->trustLineReadOnly(contractorUUID);
    ss << contractorUUID << "\t";
    ss << kContractorTrustLine->incomingTrustAmount() << "\t";
    ss << kContractorTrustLine->outgoingTrustAmount() << "\t";
    ss << kContractorTrustLine->balance();
    ss << "\n";
    string kResultInfo = ss.str();
    return transactionResultFromCommand(
        mCommand->resultOk(
            kResultInfo));
}

TransactionResult::SharedConst GetFirstLevelContractorBalanceTransaction::resultTrustLineIsAbsent()
{
    return transactionResultFromCommand(
        mCommand->responseTrustlineIsAbsent());
}