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
    mStorageHandler(storageHandler),
    mCountSendingAttempts(0)
{}

TransactionResult::SharedConst SetChannelContractorCryptoKeyTransaction::run()
{
    info() << "step " << mStep;
    switch (mStep) {
        case Stages::Initialization: {
            return runInitializationStage();
        }
        case Stages::ResponseProcessing: {
            return runResponseProcessingStage();
        }
        default:
            throw ValueError(logHeader() + "::run: wrong value of mStep");
    }
}

TransactionResult::SharedConst SetChannelContractorCryptoKeyTransaction::runInitializationStage()
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

    auto contractor = mContractorsManager->contractor(
        mCommand->contractorChannelID());
    sendMessage<InitChannelMessage>(
        contractor->getID(),
        mContractorsManager->ownAddresses(),
        mTransactionUUID,
        contractor);
    mStep = ResponseProcessing;
    return resultOKAndWaitResponse();
}

TransactionResult::SharedConst SetChannelContractorCryptoKeyTransaction::runResponseProcessingStage()
{
    if (mContext.empty()) {
        warning() << "Contractor don't send response.";
        if (mCountSendingAttempts < kMaxCountSendingAttempts) {
            auto contractor = mContractorsManager->contractor(
                mCommand->contractorChannelID());
            sendMessage<InitChannelMessage>(
                contractor->getID(),
                mContractorsManager->ownAddresses(),
                mTransactionUUID,
                contractor);
            mCountSendingAttempts++;
            info() << "Send message " << mCountSendingAttempts << " times";
            return resultWaitForMessageTypes(
                {Message::Channel_Confirm},
                kWaitMillisecondsForResponse);
        }
        info() << "Channel was not confirm";
        return resultDone();
    }
    auto message = popNextMessage<ConfirmChannelMessage>();
    // todo : check response
    info() << "contractor " << message->idOnReceiverSide << " send channel confirmation";
    return resultDone();
}

TransactionResult::SharedConst SetChannelContractorCryptoKeyTransaction::resultOKAndWaitResponse()
{
    return transactionResultFromCommandAndWaitForMessageTypes(
        mCommand->responseOK(),
        {Message::Channel_Confirm},
        kWaitMillisecondsForResponse);
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