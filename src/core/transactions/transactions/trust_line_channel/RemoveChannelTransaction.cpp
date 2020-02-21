#include "RemoveChannelTransaction.h"

RemoveChannelTransaction::RemoveChannelTransaction(
    RemoveChannelCommand::Shared command,
    ContractorsManager *contractorsManager,
    StorageHandler *storageHandler,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::RemoveChannel,
        0,
        logger),
    mCommand(command),
    mContractorsManager(contractorsManager),
    mStorageHandler(storageHandler)
{}

TransactionResult::SharedConst RemoveChannelTransaction::run()
{
    info() << "Try remove channel with " << mCommand->contractorChannelID();
    if (!mContractorsManager->contractorPresent(mCommand->contractorChannelID())) {
        warning() << "There is no contractor with requested id";
        return resultContractorIsAbsent();
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        mContractorsManager->removeContractor(
            ioTransaction,
            mCommand->contractorChannelID());
    } catch(ValueError &e) {
        ioTransaction->rollback();
        warning() << "Attempt to remove contractor " << mCommand->contractorChannelID() << " failed. "
                  << "Details are: " << e.what();
        return resultProtocolError();
    } catch(IOError &e) {
        ioTransaction->rollback();
        warning() << "Attempt to remove contractor " << mCommand->contractorChannelID() << " failed. "
                  << "Details are: " << e.what();
        return resultUnexpectedError();
    }
    info() << "Channel successfully removed";

    return resultOK();
}

TransactionResult::SharedConst RemoveChannelTransaction::resultOK()
{
    return transactionResultFromCommand(
        mCommand->responseOK());
}

TransactionResult::SharedConst RemoveChannelTransaction::resultProtocolError()
{
    return transactionResultFromCommand(
        mCommand->responseProtocolError());
}

TransactionResult::SharedConst RemoveChannelTransaction::resultContractorIsAbsent()
{
    return transactionResultFromCommand(
        mCommand->responseTrustLineIsAbsent());
}

TransactionResult::SharedConst RemoveChannelTransaction::resultUnexpectedError()
{
    return transactionResultFromCommand(
        mCommand->responseUnexpectedError());
}

const string RemoveChannelTransaction::logHeader() const
{
    stringstream s;
    s << "[RemoveChannelTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}