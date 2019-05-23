#include "SetChannelContractorAddressesTransaction.h"

SetChannelContractorAddressesTransaction::SetChannelContractorAddressesTransaction(
    SetChannelContractorAddressesCommand::Shared command,
    ContractorsManager *contractorsManager,
    StorageHandler *storageHandler,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::SetChannelContractorAddresses,
        0,
        logger),
    mCommand(command),
    mContractorsManager(contractorsManager),
    mStorageHandler(storageHandler)
{}

TransactionResult::SharedConst SetChannelContractorAddressesTransaction::run()
{
    info() << "Try set channel addresses to Contractor: " << mCommand->contractorChannelID();
    info() << "New addresses:";
    for (const auto &address : mCommand->contractorAddresses()) {
        info() << address->fullAddress();
    }
    if (mCommand->contractorAddresses().empty()) {
        warning() << "Contractor addresses are empty";
        return resultProtocolError();
    }
    if (!mContractorsManager->contractorPresent(mCommand->contractorChannelID())) {
        warning() << "There is no contractor with requested id ";
        return resultContractorIsAbsent();
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        mContractorsManager->updateContractorAddresses(
            ioTransaction,
            mCommand->contractorChannelID(),
            mCommand->contractorAddresses());
        info() << "Addresses updated";
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Error during saving data into database. Details: " << e.what();
        return resultUnexpectedError();
    }
    return resultOK();
}

TransactionResult::SharedConst SetChannelContractorAddressesTransaction::resultOK()
{
    return transactionResultFromCommand(
       mCommand->responseOK());
}

TransactionResult::SharedConst SetChannelContractorAddressesTransaction::resultProtocolError()
{
    return transactionResultFromCommand(
        mCommand->responseProtocolError());
}

TransactionResult::SharedConst SetChannelContractorAddressesTransaction::resultContractorIsAbsent()
{
    return transactionResultFromCommand(
        mCommand->responseTrustLineIsAbsent());
}

TransactionResult::SharedConst SetChannelContractorAddressesTransaction::resultUnexpectedError()
{
    return transactionResultFromCommand(
        mCommand->responseUnexpectedError());
}

const string SetChannelContractorAddressesTransaction::logHeader() const
{
    stringstream s;
    s << "[SetChannelContractorAddressesTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}