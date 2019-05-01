#include "GetChannelInfoTransaction.h"

GetChannelInfoTransaction::GetChannelInfoTransaction(
    GetChannelInfoCommand::Shared command,
    ContractorsManager *contractorsManager,
    Logger &logger):
    BaseTransaction(
        BaseTransaction::ChannelInfo,
        0,
        logger),
    mCommand(command),
    mContractorsManager(contractorsManager)
{}

TransactionResult::SharedConst GetChannelInfoTransaction::run()
{
    if (!mContractorsManager->contractorPresent(mCommand->contractorID())) {
        return resultContractorIsAbsent();
    }
    stringstream ss;
    auto contractor = mContractorsManager->contractor(
        mCommand->contractorID());
    ss << contractor->getID() << kTokensSeparator
       << contractor->addresses().size() << kTokensSeparator;
    for (const auto &address : contractor->addresses()) {
        ss << address->fullAddress() << kTokensSeparator;
    }
    ss << contractor->isConfirmed() << kTokensSeparator;
    ss << *contractor->cryptoKey()->publicKey << kCommandsSeparator;
    string kResultInfo = ss.str();
    return transactionResultFromCommand(
        mCommand->resultOk(
            kResultInfo));
}

TransactionResult::SharedConst GetChannelInfoTransaction::resultContractorIsAbsent()
{
    return transactionResultFromCommand(
        mCommand->responseTrustLineIsAbsent());
}

const string GetChannelInfoTransaction::logHeader() const
{
    stringstream s;
    s << "[GetChannelInfoTA: " << currentTransactionUUID() << " " << mEquivalent << "] ";
    return s.str();
}