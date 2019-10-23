#include "RemoveTrustLineTransaction.h"

RemoveTrustLineTransaction::RemoveTrustLineTransaction(
    RemoveTrustLineCommand::Shared command,
    ContractorsManager *contractorsManager,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::RemoveTrustLineTransactionType,
            command->equivalent(),
            logger),
    mCommand(command),
    mContractorID(command->contractorID()),
    mContractorsManager(contractorsManager),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mKeysStore(keystore)
{}

TransactionResult::SharedConst RemoveTrustLineTransaction::run()
{
    info() << "try remove TL with " << mContractorID;
    if (!mContractorsManager->contractorPresent(mContractorID)) {
        warning() << "There is no contractor with requested id";
        return resultProtocolError();
    }

    try {
        if (mTrustLines->trustLineState(mContractorID) != TrustLine::Archived and
                mTrustLines->trustLineState(mContractorID) != TrustLine::Init) {
            warning() << "Invalid TL state " << mTrustLines->trustLineState(mContractorID);
            return resultProtocolError();
        }

    } catch (NotFoundError &e) {
        warning() << "Attempt to remove not existing TL";
        return resultProtocolError();
    }

    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorID));

    auto ioTransaction = mStorageHandler->beginTransaction();

    try {
        keyChain.removeAllTrustLineData(ioTransaction);
        mTrustLines->removeTrustLine(
            mContractorID,
            ioTransaction);
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Can't remove TL data. Details " << e.what();
        throw e;
    }
    info() << "TL was successfully deleted";

    return resultOK();
}

TransactionResult::SharedConst RemoveTrustLineTransaction::resultOK()
{
    return transactionResultFromCommand(
        mCommand->responseOK());
}

TransactionResult::SharedConst RemoveTrustLineTransaction::resultProtocolError()
{
    return transactionResultFromCommand(
        mCommand->responseProtocolError());
}

const string RemoveTrustLineTransaction::logHeader() const
{
    stringstream s;
    s << "[RemoveTrustLineTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}