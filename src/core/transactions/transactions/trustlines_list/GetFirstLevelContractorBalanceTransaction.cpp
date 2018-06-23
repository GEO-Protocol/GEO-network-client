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
        command->equivalent(),
        logger),
    mCommand(command),
    mTrustLinesManager(manager)
{}

GetTrustLineCommand::Shared GetFirstLevelContractorBalanceTransaction::command() const
{
    return mCommand;
}

TransactionResult::SharedConst GetFirstLevelContractorBalanceTransaction::run()
{
    stringstream ss;
    auto contractorUUID = mCommand->contractorUUID();
    if (!mTrustLinesManager->trustLineIsPresent(contractorUUID)) {
        return resultTrustLineIsAbsent();
    }
    auto kContractorTrustLine = mTrustLinesManager->trustLineReadOnly(contractorUUID);
    ss << contractorUUID << kTokensSeparator;
    ss << kContractorTrustLine->incomingTrustAmount() << kTokensSeparator;
    ss << kContractorTrustLine->outgoingTrustAmount() << kTokensSeparator;
    ss << kContractorTrustLine->balance();
    ss << kCommandsSeparator;
    string kResultInfo = ss.str();
    // todo return also state of TL
    return transactionResultFromCommand(
        mCommand->resultOk(
            kResultInfo));
}

TransactionResult::SharedConst GetFirstLevelContractorBalanceTransaction::resultTrustLineIsAbsent()
{
    return transactionResultFromCommand(
        mCommand->responseTrustLineIsAbsent());
}

const string GetFirstLevelContractorBalanceTransaction::logHeader() const
{
    stringstream s;
    s << "[GetFirstLevelContractorBalanceTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}