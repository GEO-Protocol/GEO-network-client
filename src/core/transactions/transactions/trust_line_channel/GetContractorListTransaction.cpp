#include "GetContractorListTransaction.h"

GetContractorListTransaction::GetContractorListTransaction(
    ContractorListCommand::Shared command,
    ContractorsManager *contractorsManager,
    Logger &logger):
    BaseTransaction(
        BaseTransaction::ContractorsList,
        0,
        logger),
    mCommand(command),
    mContractorsManager(contractorsManager)
{}

TransactionResult::SharedConst GetContractorListTransaction::run()
{
    const auto &contractors = mContractorsManager->allContractors();
    stringstream ss;
    ss << to_string(contractors.size());
    for (const auto &contractor: contractors) {
        ss << kTokensSeparator << contractor.get()->getID()
           << kTokensSeparator << contractor.get()->outputString();
    }
    ss << kCommandsSeparator;
    string kResultInfo = ss.str();
    return transactionResultFromCommand(
        mCommand->resultOk(
            kResultInfo));
}

const string GetContractorListTransaction::logHeader() const
{
    stringstream s;
    s << "[GetContractorListTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}