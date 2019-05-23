#include "SetChannelContractorCryptoKeyTransaction.h"

SetChannelContractorCryptoKeyTransaction::SetChannelContractorCryptoKeyTransaction(
    SetChannelContractorCryptoKeyCommand::Shared command,
    ContractorsManager *contractorsManager,
    StorageHandler *storageHandler,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::SetChannelContractorCryptoKey,
        0,
        logger),
    mCommand(command),
    mContractorsManager(contractorsManager),
    mStorageHandler(storageHandler)
{}

TransactionResult::SharedConst SetChannelContractorCryptoKeyTransaction::run()
{
    info() << "Try set channel crypto key to Contractor: " << mCommand->contractorChannelID();
    info() << "New crypto key: " << mCommand->cryptoKey();

    // todo : check crypto key length

    if (!mContractorsManager->contractorPresent(mCommand->contractorChannelID())) {
        warning() << "There is no contractor with requested id ";
        return resultContractorIsAbsent();
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        mContractorsManager->updateContractorCryptoKey(
            ioTransaction,
            mCommand->contractorChannelID(),
            mCommand->cryptoKey());
        info() << "Crypto key updated";

        // todo : send InitChannelMessage
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Error during saving data into database. Details: " << e.what();
        return resultUnexpectedError();
    }
    return resultOK();
}

TransactionResult::SharedConst SetChannelContractorCryptoKeyTransaction::resultOK()
{
    return transactionResultFromCommand(
        mCommand->responseOK());
}

TransactionResult::SharedConst SetChannelContractorCryptoKeyTransaction::resultProtocolError()
{
    return transactionResultFromCommand(
        mCommand->responseProtocolError());
}

TransactionResult::SharedConst SetChannelContractorCryptoKeyTransaction::resultContractorIsAbsent()
{
    return transactionResultFromCommand(
        mCommand->responseTrustLineIsAbsent());
}

TransactionResult::SharedConst SetChannelContractorCryptoKeyTransaction::resultUnexpectedError()
{
    return transactionResultFromCommand(
        mCommand->responseUnexpectedError());
}

const string SetChannelContractorCryptoKeyTransaction::logHeader() const
{
    stringstream s;
    s << "[SetChannelContractorCryptoKeyTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}