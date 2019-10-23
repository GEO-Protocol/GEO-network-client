#include "GetChannelInfoByAddressesTransaction.h"

GetChannelInfoByAddressesTransaction::GetChannelInfoByAddressesTransaction(
    GetChannelInfoByAddressesCommand::Shared command,
    ContractorsManager *contractorsManager,
    Logger &logger):

    BaseTransaction(
        BaseTransaction::ChannelInfoByAddresses,
        0,
        logger),
    mCommand(command),
    mContractorsManager(contractorsManager)
{}

TransactionResult::SharedConst GetChannelInfoByAddressesTransaction::run()
{
    Contractor::Shared contractorResult = nullptr;
    auto contractorAddresses = mCommand->contractorAddresses();
    for (const auto &contractor : mContractorsManager->allContractors()) {
        if (contractor->containsAddresses(contractorAddresses)) {
            contractorResult = contractor;
            break;
        }
    }
    if (contractorResult == nullptr) {
        return resultChannelIsAbsent();
    }

    stringstream ss;
    ss << contractorResult->getID() << kTokensSeparator
       << contractorResult->isConfirmed() << kCommandsSeparator;

    string kResultInfo = ss.str();
    return transactionResultFromCommand(
        mCommand->resultOk(
            kResultInfo));
}

TransactionResult::SharedConst GetChannelInfoByAddressesTransaction::resultChannelIsAbsent()
{
    return transactionResultFromCommand(
        mCommand->responseTrustLineIsAbsent());
}

const string GetChannelInfoByAddressesTransaction::logHeader() const
{
    stringstream s;
    s << "[GetChannelInfoByAddressesTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}