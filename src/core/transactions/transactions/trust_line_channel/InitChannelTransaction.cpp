#include "InitChannelTransaction.h"

InitChannelTransaction::InitChannelTransaction(
    InitChannelCommand::Shared command,
    ContractorsManager *contractorsManager,
    StorageHandler *storageHandler,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::OpenChannelTransaction,
        0,
        logger),
    mCommand(command),
    mContractorsManager(contractorsManager),
    mStorageHandler(storageHandler)
{}

TransactionResult::SharedConst InitChannelTransaction::run()
{
    info() << "Try init channel to Contractor on addresses:";
    for (const auto &address : mCommand->contractorAddresses()) {
        info() << address->fullAddress();
    }
    if (mCommand->contractorAddresses().empty()) {
        warning() << "Contractor addresses are empty";
        return resultProtocolError();
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        if (mCommand->cryptoKey().empty()) {
            info() << "Channel crypto key generation";
            mContractor = mContractorsManager->createContractor(
                ioTransaction,
                mCommand->contractorAddresses());
            info() << "Init channel to contractor with ID " << mContractor->getID()
                   << " and crypto key " << *mContractor->cryptoKey()->publicKey;
        } else {
            info() << "Channel crypto key: " << mCommand->cryptoKey();
            mContractor = mContractorsManager->createContractor(
                ioTransaction,
                mCommand->contractorAddresses(),
                mCommand->cryptoKey(),
                mCommand->contractorChannelID());
            info() << "Init channel to contractor with ID " << mContractor->getID();
            info() << "Channel ID on contractor side " << mContractor->ownIdOnContractorSide();
            sendMessage<InitChannelMessage>(
                mContractor->getID(),
                mContractorsManager->ownAddresses(),
                mTransactionUUID,
                mContractor);
        }
    } catch (ValueError &e) {
        error() << "Can't create channel. Details: " << e.what();
        return resultProtocolError();
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Error during creating channel. Details: " << e.what();
        return resultUnexpectedError();
    }
    return resultOK();
}

TransactionResult::SharedConst InitChannelTransaction::resultOK()
{
    stringstream ss;
    ss << mContractor->getID() << kTokensSeparator << *mContractor->cryptoKey()->publicKey;
    auto channelInfo = ss.str();
    return transactionResultFromCommand(
        mCommand->responseOk(channelInfo));
}

TransactionResult::SharedConst InitChannelTransaction::resultForbiddenRun()
{
    return transactionResultFromCommand(
        mCommand->responseForbiddenRunTransaction());
}

TransactionResult::SharedConst InitChannelTransaction::resultProtocolError()
{
    return transactionResultFromCommand(
        mCommand->responseProtocolError());
}

TransactionResult::SharedConst InitChannelTransaction::resultUnexpectedError()
{
    return transactionResultFromCommand(
        mCommand->responseUnexpectedError());
}

const string InitChannelTransaction::logHeader() const
{
    stringstream s;
    s << "[InitChannelTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}