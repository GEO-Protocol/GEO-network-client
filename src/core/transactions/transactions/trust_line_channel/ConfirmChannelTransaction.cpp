#include "ConfirmChannelTransaction.h"

ConfirmChannelTransaction::ConfirmChannelTransaction(
    InitChannelMessage::Shared message,
    ContractorsManager *contractorsManager,
    StorageHandler *storageHandler,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::ConfirmChannelTransaction,
        message->transactionUUID(),
        0,
        logger),
    mMessage(message),
    mContractorsManager(contractorsManager),
    mStorageHandler(storageHandler)
{}

TransactionResult::SharedConst ConfirmChannelTransaction::run()
{
    info() << "sender incoming IP " << mMessage->senderIncomingIP();
    for (auto &senderAddress : mMessage->senderAddresses) {
        info() << "contractor address " << senderAddress->fullAddress();
    }

    if (mMessage->senderAddresses.empty()) {
        warning() << "Contractor addresses are empty";
        // todo send reply
        return resultDone();
    }

    auto contractorID = mContractorsManager->contractorIDByAddresses(
        mMessage->senderAddresses);
    if (contractorID == ContractorsManager::kNotFoundContractorID) {
        warning() << "There is no contractor for requested addresses";
        // todo send reply
        return resultDone();
    }
    info() << "Channel ID " << contractorID;

    if (mContractorsManager->channelConfirmed(contractorID)) {
        info() << "Channel already confirmed";
        sendMessage<ConfirmChannelMessage>(
            contractorID,
            mTransactionUUID,
            mContractorsManager->contractor(contractorID));
        return resultDone();
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        mContractorsManager->setConfirmationInfo(
            ioTransaction,
            contractorID,
            mMessage->contractorID(),
            mMessage->publicKey());
    } catch (IOError &e) {
        error() << "Error during saving Contractor data. Details: " << e.what();
        ioTransaction->rollback();
        throw e;
    }
    info() << "Channel confirmed";
    sendMessage<ConfirmChannelMessage>(
        contractorID,
        mTransactionUUID,
        mContractorsManager->contractor(contractorID));
    return resultDone();
}

const string ConfirmChannelTransaction::logHeader() const
{
    stringstream s;
    s << "[ConfirmChannelTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}