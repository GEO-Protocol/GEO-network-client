#include "GetTrustLineByAddressTransaction.h"

GetTrustLineByAddressTransaction::GetTrustLineByAddressTransaction(
    GetTrustLineByAddressCommand::Shared command,
    ContractorsManager *contractorsManager,
    TrustLinesManager *trustLinesManager,
    Logger &logger)
    noexcept:
    BaseTransaction(
        BaseTransaction::TrustLineOneByAddress,
        command->equivalent(),
        logger),
    mCommand(command),
    mContractorsManager(contractorsManager),
    mTrustLinesManager(trustLinesManager)
{}

TransactionResult::SharedConst GetTrustLineByAddressTransaction::run()
{
    auto contractorID = ContractorsManager::kNotFoundContractorID;
    auto contractorAddresses = mCommand->contractorAddresses();
    for (const auto &contractor : mContractorsManager->allContractors()) {
        if (contractor->containsAddresses(contractorAddresses)) {
            contractorID = contractor->getID();
            break;
        }
    }
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
    return transactionResultFromCommand(
        mCommand->resultOk(
            kResultInfo));
}

TransactionResult::SharedConst GetTrustLineByAddressTransaction::resultTrustLineIsAbsent()
{
    return transactionResultFromCommand(
        mCommand->responseTrustLineIsAbsent());
}

const string GetTrustLineByAddressTransaction::logHeader() const
{
    stringstream s;
    s << "[GetTrustLineByAddressTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}