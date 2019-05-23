#include "RegenerateChannelCryptoKeyTransaction.h"

RegenerateChannelCryptoKeyTransaction::RegenerateChannelCryptoKeyTransaction(
    RegenerateChannelCryptoKeyCommand::Shared command,
    ContractorsManager *contractorsManager,
    StorageHandler *storageHandler,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::RegenerateChannelCryptoKey,
        0,
        logger),
    mCommand(command),
    mContractorsManager(contractorsManager),
    mStorageHandler(storageHandler)
{}

TransactionResult::SharedConst RegenerateChannelCryptoKeyTransaction::run()
{
    info() << "Regenerate crypto key for channel: " << mCommand->contractorChannelID();
    if (!mContractorsManager->contractorPresent(mCommand->contractorChannelID())) {
        warning() << "There is no contractor with requested id ";
        return resultContractorIsAbsent();
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        mContractorsManager->regenerateCryptoKey(
            ioTransaction,
            mCommand->contractorChannelID());
        info() << "Regenerated crypto key "
               << *mContractorsManager->contractor(mCommand->contractorChannelID())->cryptoKey()->publicKey;
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Error during saving channel's updated data. Details: " << e.what();
        return resultUnexpectedError();
    }
    return resultOK();
}

TransactionResult::SharedConst RegenerateChannelCryptoKeyTransaction::resultOK()
{
    stringstream ss;
    ss << mCommand->contractorChannelID() << kTokensSeparator
       << *mContractorsManager->contractor(mCommand->contractorChannelID())->cryptoKey()->publicKey;
    auto channelInfo = ss.str();
    return transactionResultFromCommand(
        mCommand->responseOk(channelInfo));
}

TransactionResult::SharedConst RegenerateChannelCryptoKeyTransaction::resultForbiddenRun()
{
    return transactionResultFromCommand(
        mCommand->responseForbiddenRunTransaction());
}

TransactionResult::SharedConst RegenerateChannelCryptoKeyTransaction::resultContractorIsAbsent()
{
    return transactionResultFromCommand(
        mCommand->responseTrustLineIsAbsent());
}

TransactionResult::SharedConst RegenerateChannelCryptoKeyTransaction::resultProtocolError()
{
    return transactionResultFromCommand(
        mCommand->responseProtocolError());
}

TransactionResult::SharedConst RegenerateChannelCryptoKeyTransaction::resultUnexpectedError()
{
    return transactionResultFromCommand(
        mCommand->responseUnexpectedError());
}

const string RegenerateChannelCryptoKeyTransaction::logHeader() const
{
    stringstream s;
    s << "[RegenerateChannelCryptoKeyTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}