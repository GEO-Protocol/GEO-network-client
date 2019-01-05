#include "GetFirstLevelContractorBalanceTransaction.h"

GetFirstLevelContractorBalanceTransaction::GetFirstLevelContractorBalanceTransaction(
    GetTrustLineCommand::Shared command,
    ContractorsManager *contractorsManager,
    TrustLinesManager *trustLinesManager,
    Logger &logger)
noexcept:
    BaseTransaction(
        BaseTransaction::TrustLineOne,
        command->equivalent(),
        logger),
    mCommand(command),
    mContractorsManager(contractorsManager),
    mTrustLinesManager(trustLinesManager)
{}

TransactionResult::SharedConst GetFirstLevelContractorBalanceTransaction::run()
{
    auto contractorID = mContractorsManager->contractorIDByAddress(
        mCommand->contractorAddress());
    if (contractorID == ContractorsManager::kNotFoundContractorID) {
        return resultTrustLineIsAbsent();
    }
    if (!mTrustLinesManager->trustLineIsPresent(contractorID)) {
        return resultTrustLineIsAbsent();
    }
    stringstream ss;
    auto kContractorTrustLine = mTrustLinesManager->trustLineReadOnly(contractorID);
    ss << contractorID << kTokensSeparator;
    ss << kContractorTrustLine->state() << kTokensSeparator;
    ss << kContractorTrustLine->isOwnKeysPresent() << kTokensSeparator;
    ss << kContractorTrustLine->isContractorKeysPresent() << kTokensSeparator;
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