#include "GetFirstLevelContractorsTransaction.h"

GetFirstLevelContractorsTransaction::GetFirstLevelContractorsTransaction(
    GetFirstLevelContractorsCommand::Shared command,
    TrustLinesManager *trustLinesManager,
    ContractorsManager *contractorsManager,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::ContractorsList,
        command->equivalent(),
        logger),
    mCommand(command),
    mTrustLinesManager(trustLinesManager),
    mContractorsManager(contractorsManager)
{}

TransactionResult::SharedConst GetFirstLevelContractorsTransaction::run()
{
    info() << "run";
    const auto neighborsCount = mTrustLinesManager->trustLines().size();
    stringstream ss;
    ss << to_string(neighborsCount);
    for (const auto &nodeIDAndTrustLine: mTrustLinesManager->trustLines()) {
        ss << kTokensSeparator << nodeIDAndTrustLine.first
           << kTokensSeparator << mContractorsManager->contractor(nodeIDAndTrustLine.first)->outputString();
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
